## @file
# @details A GOsa-SI event module containing all functions common used by GOsa
# @brief Implementation of a GOsa-SI event module. 

package gosaTriggered;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    "get_events", 
    "get_login_usr_for_client",
    "get_client_for_login_usr",
    "gen_smb_hash",
    "trigger_reload_syslog_config",
    "trigger_reload_ntp_config",
    "trigger_reload_ldap_config",
    "ping",
    "network_completition",
    "set_activated_for_installation",
    "new_key_for_client",
    "detect_hardware",
    "get_login_usr",
    "get_login_client",
    "trigger_action_localboot",
    "trigger_action_faireboot",
    "trigger_action_reboot",
    "trigger_action_activate",
    "trigger_action_lock",
    "trigger_action_halt",
    "trigger_action_update", 
    "trigger_action_reinstall",
    "trigger_action_memcheck", 
    "trigger_action_sysinfo",
    "trigger_action_instant_update",
    "trigger_action_rescan",
    "trigger_action_wake",
    "recreate_fai_server_db",
    "recreate_fai_release_db",
    "recreate_packages_list_db",
    "send_user_msg", 
    "get_available_kernel",
	"trigger_activate_new",
    "get_hosts_with_module",    
#	"get_dak_keyring",
#	"import_dak_key",
#	"remove_dak_key",
#    "get_dak_queue",
    );
@EXPORT = @events;

use strict;
use warnings;
use GOSA::GosaSupportDaemon;
use Data::Dumper;
use Crypt::SmbHash;
use Net::ARP;
use Net::Ping;
use Socket;
use Time::HiRes qw( usleep);
use MIME::Base64;

BEGIN {}

END {}

### Start ######################################################################

## @method get_events()
# A brief function returning a list of functions which are exported by importing the module.
# @return List of all provided functions
sub get_events {
    return \@events;
}

## @method send_usr_msg($msg, $msg_hash, $session_id)
# This function accepts usr messages from GOsa, split mulitple target messages to mulitiple single target messages and put all messages into messaging_db
# @param msg - STRING - xml message
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return (out_msg)  - ARRAY - Array containing the answer message from client
sub send_user_msg {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];

    #my $subject = &decode_base64(@{$msg_hash->{'subject'}}[0]);   # just for debugging
    my $subject = @{$msg_hash->{'subject'}}[0];
    my $from = @{$msg_hash->{'from'}}[0];
    my @users = exists $msg_hash->{'user'} ? @{$msg_hash->{'user'}} : () ;
	my @groups = exists $msg_hash->{'group'} ? @{$msg_hash->{'group'}} : ();
    my $delivery_time = @{$msg_hash->{'delivery_time'}}[0];
    #my $message = &decode_base64(@{$msg_hash->{'message'}}[0]);   # just for debugging
    my $message = @{$msg_hash->{'message'}}[0];
    
    # keep job queue uptodate if necessary 
    my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
    if( defined $jobdb_id) {
        my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=$jobdb_id";
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7); 
        my $res = $main::job_db->exec_statement($sql_statement);
    }

    # error handling
    if (not $delivery_time =~ /^\d{14}$/) {
        my $error_string = "delivery_time '$delivery_time' is not a valid timestamp, please use format 'yyyymmddhhmmss'";
        &main::daemon_log("$session_id ERROR: $error_string", 1);
        return &create_xml_string(&create_xml_hash($header, $target, $source, $error_string));
    }

    # determine new message id
    my $new_msg_id = 1;
	my $new_msg_id_sql = "SELECT MAX(id) FROM $main::messaging_tn";
    my $new_msg_id_res = $main::messaging_db->exec_statement($new_msg_id_sql);
    if (defined @{@{$new_msg_id_res}[0]}[0] ) {
        $new_msg_id = int(@{@{$new_msg_id_res}[0]}[0]);
        $new_msg_id += 1;
    }

	# highlight user name and group name
	my @receiver_l;
	@users = map(push(@receiver_l, "u_$_"), @users);
	@groups = map(push(@receiver_l, "g_$_"), @groups);

    # Sanitiy check of receivers list
    if (@receiver_l == 0) {
        &main::daemon_log("$session_id ERROR: 'send_usr_msg'-message contains neither a 'usr' nor a 'group' tag. No receiver specified.", 1); 
        return;
    }

    # add incoming message to messaging_db
    my $func_dic = {table=>$main::messaging_tn,
        primkey=>[],
        id=>$new_msg_id,
        subject=>$subject,
        message_from=>$from,
        message_to=>join(",", @receiver_l),
        flag=>"n",
        direction=>"in",
        delivery_time=>$delivery_time,
        message=>$message,
        timestamp=>&get_time(),
    };
    my $res = $main::messaging_db->add_dbentry($func_dic);
    if (not $res == 0) {
        &main::daemon_log("$session_id ERROR: gosaTriggered.pm: cannot add message to message_db: $res", 1);
    } else {
        &main::daemon_log("$session_id INFO: gosaTriggered.pm: message with subject '".&decode_base64($subject)."' successfully added to message_db", 5);
    }

    return;
}


sub recreate_fai_server_db {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $out_msg;

    my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
    if( defined $jobdb_id) {
        my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=$jobdb_id";
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7); 
        my $res = $main::job_db->exec_statement($sql_statement);
    }

    $main::fai_server_db->create_table("new_fai_server", \@main::fai_server_col_names);
    &main::create_fai_server_db("new_fai_server",undef,"dont", $session_id);
    $main::fai_server_db->move_table("new_fai_server", $main::fai_server_tn);
    
    my @out_msg_l = ( $out_msg );
    return @out_msg_l;
}


sub recreate_fai_release_db {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $out_msg;

    my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
    if( defined $jobdb_id) {
        my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=$jobdb_id";
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7);
        my $res = $main::job_db->exec_statement($sql_statement);
    }

    $main::fai_release_db->create_table("new_fai_release", \@main::fai_release_col_names);
    &main::create_fai_release_db("new_fai_release", $session_id);
    $main::fai_release_db->move_table("new_fai_release", $main::fai_release_tn);

    my @out_msg_l = ( $out_msg );
    return @out_msg_l;
}


sub recreate_packages_list_db {
	my ($msg, $msg_hash, $session_id) = @_ ;
	my $out_msg;

	my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
	if( defined $jobdb_id) {
		my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=$jobdb_id";
		&main::daemon_log("$session_id DEBUG: $sql_statement", 7);
		my $res = $main::job_db->exec_statement($sql_statement);
	}

	&main::create_packages_list_db(undef, undef, $session_id);

	my @out_msg_l = ( $out_msg );
	return @out_msg_l;
}


sub get_login_usr_for_client {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $client = @{$msg_hash->{'client'}}[0];

    my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
    if( defined $jobdb_id) {
        my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=$jobdb_id";
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7); 
        my $res = $main::job_db->exec_statement($sql_statement);
    }

    $header =~ s/^gosa_//;

    my $sql_statement = "SELECT * FROM known_clients WHERE hostname='$client' OR macaddress LIKE '$client'";
    my $res = $main::known_clients_db->select_dbentry($sql_statement);

    my $out_msg = "<xml><header>$header</header><source>$target</source><target>$source</target>";
    $out_msg .= &db_res2xml($res);
    $out_msg .= "</xml>";

    my @out_msg_l = ( $out_msg );
    return @out_msg_l;
}


sub get_client_for_login_usr {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];

    my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
    if( defined $jobdb_id) {
        my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=$jobdb_id";
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7); 
        my $res = $main::job_db->exec_statement($sql_statement);
    }

    my $usr = @{$msg_hash->{'usr'}}[0];
    $header =~ s/^gosa_//;

    my $sql_statement = "SELECT * FROM known_clients WHERE login LIKE '%$usr%'";
    my $res = $main::known_clients_db->select_dbentry($sql_statement);

    my $out_msg = "<xml><header>$header</header><source>$target</source><target>$source</target>";
    $out_msg .= &db_res2xml($res);
    $out_msg .= "</xml>";
    my @out_msg_l = ( $out_msg );
    return @out_msg_l;

}


sub ping {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $header = @{$msg_hash->{header}}[0];
    my $target = @{$msg_hash->{target}}[0];
    my $source = @{$msg_hash->{source}}[0];
    my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
    my $error = 0;
    my $answer_msg;
    my ($sql, $res);

    if( defined $jobdb_id) {
        my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=$jobdb_id";
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7); 
        my $res = $main::job_db->exec_statement($sql_statement);
    }

    # send message
    $sql = "SELECT * FROM $main::known_clients_tn WHERE ((hostname='$target') || (macaddress LIKE '$target'))"; 
    $res = $main::known_clients_db->exec_statement($sql);

    # sanity check of db result
    my ($host_name, $host_key);
    if ((defined $res) && (@$res > 0) && @{@$res[0]} > 0) {
        $host_name = @{@$res[0]}[0];
        $host_key = @{@$res[0]}[2];
    } else {
        &main::daemon_log("$session_id ERROR: cannot determine host_name and host_key from known_clients_db at function ping\n$msg", 1);
        $error = 1;
    }

    if (not $error) {
        my $client_hash = &create_xml_hash("ping", $main::server_address, $host_name);
        &add_content2xml_hash($client_hash, 'session_id', $session_id); 
        my $client_msg = &create_xml_string($client_hash);
        &main::send_msg_to_target($client_msg, $host_name, $host_key, $header, $session_id);

        my $message_id;
        my $i = 0;
        while (1) {
            $i++;
            $sql = "SELECT * FROM $main::incoming_tn WHERE headertag='answer_$session_id'";
            $res = $main::incoming_db->exec_statement($sql);
            if (ref @$res[0] eq "ARRAY") { 
                $message_id = @{@$res[0]}[0];
                last;
            }

            # do not run into a endless loop
            if ($i > 100) { last; }
            usleep(100000);
        }

        # if an answer to the question exists
        if (defined $message_id) {
            my $answer_xml = @{@$res[0]}[3];
            my %data = ( 'answer_xml'  => 'bin noch da' );
            $answer_msg = &build_msg("got_ping", $target, $source, \%data);
            my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
            if (defined $forward_to_gosa){
                $answer_msg =~s/<\/xml>/<forward_to_gosa>$forward_to_gosa<\/forward_to_gosa><\/xml>/;
            }
            $sql = "DELETE FROM $main::incoming_tn WHERE id=$message_id"; 
            $res = $main::incoming_db->exec_statement($sql);
        }

    }

    return ( $answer_msg );
}



sub gen_smb_hash {
     my ($msg, $msg_hash, $session_id) = @_ ;
     my $source = @{$msg_hash->{source}}[0];
     my $target = @{$msg_hash->{target}}[0];
     my $password = @{$msg_hash->{password}}[0];

     my %data= ('hash' => join(q[:], ntlmgen $password));
     my $out_msg = &build_msg("gen_smb_hash", $target, $source, \%data );
     my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
     if (defined $forward_to_gosa) {
         $out_msg =~s/<\/xml>/<forward_to_gosa>$forward_to_gosa<\/forward_to_gosa><\/xml>/;
     }

     return ( $out_msg );
}


sub network_completition {
     my ($msg, $msg_hash, $session_id) = @_ ;
     my $source = @{$msg_hash->{source}}[0];
     my $target = @{$msg_hash->{target}}[0];
     my $name = @{$msg_hash->{hostname}}[0];

     # Can we resolv the name?
     my %data;
     if (inet_aton($name)){
	     my $address = inet_ntoa(inet_aton($name));
	     my $p = Net::Ping->new('tcp');
	     my $mac= "";
	     if ($p->ping($address, 1)){
	       $mac = Net::ARP::arp_lookup("", $address);
	     }

	     %data= ('ip' => $address, 'mac' => $mac);
     } else {
	     %data= ('ip' => '', 'mac' => '');
     }

     my $out_msg = &build_msg("network_completition", $target, $source, \%data );
     my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
     if (defined $forward_to_gosa) {
         $out_msg =~s/<\/xml>/<forward_to_gosa>$forward_to_gosa<\/forward_to_gosa><\/xml>/;
     }

     return ( $out_msg );
}


sub detect_hardware {
    my ($msg, $msg_hash, $session_id) = @_ ;
    # just forward msg to client, but dont forget to split off 'gosa_' in header
    my $source = @{$msg_hash->{source}}[0];
    my $target = @{$msg_hash->{target}}[0];
    my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
    if( defined $jobdb_id) {
        my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=$jobdb_id";
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7); 
        my $res = $main::job_db->exec_statement($sql_statement);
    }

    my $out_hash = &create_xml_hash("detect_hardware", $source, $target);
    if( defined $jobdb_id ) { 
        &add_content2xml_hash($out_hash, 'jobdb_id', $jobdb_id); 
    }
    my $out_msg = &create_xml_string($out_hash);

    my @out_msg_l = ( $out_msg );
    return @out_msg_l;

}

sub trigger_reload_syslog_config {
    my ($msg, $msg_hash, $session_id) = @_ ;

    # Sanity check of macaddress
    # TODO

    my $macaddress = @{$msg_hash->{macaddress}}[0];

    my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
    if( defined $jobdb_id) {
        my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=$jobdb_id";
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7); 
        my $res = $main::job_db->exec_statement($sql_statement);
    }

	my $out_msg = &ClientPackages::new_syslog_config($macaddress, $session_id);
	my @out_msg_l = ( $out_msg );

    return @out_msg_l;

   
}

sub trigger_reload_ntp_config {
    my ($msg, $msg_hash, $session_id) = @_ ;

    # Sanity check of macaddress
    # TODO

    my $macaddress = @{$msg_hash->{macaddress}}[0];

    my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
    if( defined $jobdb_id) {
        my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=$jobdb_id";
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7); 
        my $res = $main::job_db->exec_statement($sql_statement);
    }

	my $out_msg = &ClientPackages::new_ntp_config($macaddress, $session_id);
	my @out_msg_l = ( $out_msg );

    return @out_msg_l;

}

sub trigger_reload_ldap_config {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $target = @{$msg_hash->{target}}[0];

    my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
    if( defined $jobdb_id) {
        my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=$jobdb_id";
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7); 
        my $res = $main::job_db->exec_statement($sql_statement);
    }

	my $out_msg = &ClientPackages::new_ldap_config($target, $session_id);
	my @out_msg_l = ( $out_msg );

    return @out_msg_l;
}


sub set_activated_for_installation {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{header}}[0];
    my $source = @{$msg_hash->{source}}[0];
    my $target = @{$msg_hash->{target}}[0];
	my $mac= (defined($msg_hash->{'macaddress'}))?@{$msg_hash->{'macaddress'}}[0]:undef;
	my @out_msg_l;

	# update status of job 
    my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
    if( defined $jobdb_id) {
        my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=$jobdb_id";
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7); 
        my $res = $main::job_db->exec_statement($sql_statement);
    }

    # If a client gets a 'set_activated_for_installation' msg, always deliver a fresh 'new_ldap_config'
    # just for backup and robustness purposes
    my $ldap_out_msg = &ClientPackages::new_ldap_config($mac, $session_id);
    push(@out_msg_l, $ldap_out_msg);

	# create set_activated_for_installation message for delivery
    my $out_hash = &create_xml_hash("set_activated_for_installation", $source, $target);
    if( defined $jobdb_id ) { 
        &add_content2xml_hash($out_hash, 'jobdb_id', $jobdb_id); 
    }
    my $out_msg = &create_xml_string($out_hash);
	push(@out_msg_l, $out_msg); 

    return @out_msg_l;
}


sub trigger_action_faireboot {
    my ($msg, $msg_hash, $session_id) = @_;
    my $macaddress = @{$msg_hash->{macaddress}}[0];
    my $source = @{$msg_hash->{source}}[0];

    my @out_msg_l;
    $msg =~ s/<header>gosa_trigger_action_faireboot<\/header>/<header>trigger_action_faireboot<\/header>/;
    push(@out_msg_l, $msg);

    &main::change_goto_state('locked', \@{$msg_hash->{macaddress}}, $session_id);
	&main::change_fai_state('install', \@{$msg_hash->{macaddress}}, $session_id); 

    # set job to status 'done', job will be deleted automatically
    my $sql_statement = "UPDATE $main::job_queue_tn ".
        "SET status='done', modified='1'".
        "WHERE (macaddress='$macaddress' AND status='processing')";
    &main::daemon_log("$session_id DEBUG: $sql_statement", 7);
    my $res = $main::job_db->update_dbentry( $sql_statement );

    return @out_msg_l;
}


sub trigger_action_lock {
    my ($msg, $msg_hash, $session_id) = @_;
    my $macaddress = @{$msg_hash->{macaddress}}[0];
    my $source = @{$msg_hash->{source}}[0];

    &main::change_goto_state('locked', \@{$msg_hash->{macaddress}}, $session_id);
    my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
    if( defined $jobdb_id) {
        my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=$jobdb_id";
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7); 
        my $res = $main::job_db->exec_statement($sql_statement);
    }
                                             
    my @out_msg_l;
    return @out_msg_l;
}


sub trigger_action_activate {
    my ($msg, $msg_hash, $session_id) = @_;
    my $macaddress = @{$msg_hash->{macaddress}}[0];
    my $source = @{$msg_hash->{source}}[0];

    &main::change_goto_state('active', \@{$msg_hash->{macaddress}}, $session_id);
    my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
    if( defined $jobdb_id) {
        my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=$jobdb_id";
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7); 
        my $res = $main::job_db->exec_statement($sql_statement);
    }
                                             
    my $out_hash = &create_xml_hash("set_activated_for_installation", $source, $macaddress);
    if( exists $msg_hash->{'jobdb_id'} ) { 
        &add_content2xml_hash($out_hash, 'jobdb_id', @{$msg_hash->{'jobdb_id'}}[0]); 
    }
    my $out_msg = &create_xml_string($out_hash);

    my @out_msg_l = ($out_msg);  
    return @out_msg_l;

}


sub trigger_action_localboot {
    my ($msg, $msg_hash, $session_id) = @_;
    $msg =~ s/<header>gosa_trigger_action_localboot<\/header>/<header>trigger_action_localboot<\/header>/;
    &main::change_fai_state('localboot', \@{$msg_hash->{macaddress}}, $session_id);
    my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
    if( defined $jobdb_id) {
        my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=$jobdb_id";
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7); 
        my $res = $main::job_db->exec_statement($sql_statement);
    }

    my @out_msg_l = ($msg);  
    return @out_msg_l;
}


sub trigger_action_halt {
    my ($msg, $msg_hash, $session_id) = @_;
    $msg =~ s/<header>gosa_trigger_action_halt<\/header>/<header>trigger_action_halt<\/header>/;

    my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
    if( defined $jobdb_id) {
        my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=$jobdb_id";
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7); 
        my $res = $main::job_db->exec_statement($sql_statement);
    }

    my @out_msg_l = ($msg);  
    return @out_msg_l;
}


sub trigger_action_reboot {
    my ($msg, $msg_hash, $session_id) = @_;
    $msg =~ s/<header>gosa_trigger_action_reboot<\/header>/<header>trigger_action_reboot<\/header>/;

    &main::change_fai_state('reboot', \@{$msg_hash->{macaddress}}, $session_id);
    my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
    if( defined $jobdb_id) {
        my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=$jobdb_id";
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7); 
        my $res = $main::job_db->exec_statement($sql_statement);
    }

    my @out_msg_l = ($msg);  
    return @out_msg_l;
}


sub trigger_action_memcheck {
    my ($msg, $msg_hash, $session_id) = @_ ;
    $msg =~ s/<header>gosa_trigger_action_memcheck<\/header>/<header>trigger_action_memcheck<\/header>/;

    &main::change_fai_state('memcheck', \@{$msg_hash->{macaddress}}, $session_id);
    my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
    if( defined $jobdb_id) {
        my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=$jobdb_id";
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7); 
        my $res = $main::job_db->exec_statement($sql_statement);
    }

    my @out_msg_l = ($msg);  
    return @out_msg_l;
}


sub trigger_action_reinstall {
    my ($msg, $msg_hash, $session_id) = @_;
    $msg =~ s/<header>gosa_trigger_action_reinstall<\/header>/<header>trigger_action_reinstall<\/header>/;

    &main::change_fai_state('reinstall', \@{$msg_hash->{macaddress}}, $session_id);

    my %data = ( 'macaddress'  => \@{$msg_hash->{macaddress}} );
    my $wake_msg = &build_msg("trigger_wake", "GOSA", "KNOWN_SERVER", \%data);
    # invoke trigger wake for this gosa-si-server
    &main::server_server_com::trigger_wake($msg, $msg_hash, $session_id);

    my @out_msg_l = ($wake_msg, $msg);  
    return @out_msg_l;
}


sub trigger_action_update {
    my ($msg, $msg_hash, $session_id) = @_;
    $msg =~ s/<header>gosa_trigger_action_update<\/header>/<header>trigger_action_update<\/header>/;

    &main::change_fai_state('update', \@{$msg_hash->{macaddress}}, $session_id);

    my %data = ( 'macaddress'  => \@{$msg_hash->{macaddress}} );
    my $wake_msg = &build_msg("trigger_wake", "GOSA", "KNOWN_SERVER", \%data);
    # invoke trigger wake for this gosa-si-server
    &main::server_server_com::trigger_wake($msg, $msg_hash, $session_id);

    my @out_msg_l = ($wake_msg, $msg);  
    return @out_msg_l;
}


sub trigger_action_instant_update {
    my ($msg, $msg_hash, $session_id) = @_;
    $msg =~ s/<header>gosa_trigger_action_instant_update<\/header>/<header>trigger_action_instant_update<\/header>/;

    &main::change_fai_state('update', \@{$msg_hash->{macaddress}}, $session_id);

    my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
    if( defined $jobdb_id) {
        my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=$jobdb_id";
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7); 
        my $res = $main::job_db->exec_statement($sql_statement);
    }


    my %data = ( 'macaddress'  => \@{$msg_hash->{macaddress}} );
    my $wake_msg = &build_msg("trigger_wake", "GOSA", "KNOWN_SERVER", \%data);
    # invoke trigger wake for this gosa-si-server
    &main::server_server_com::trigger_wake($msg, $msg_hash, $session_id);

    my @out_msg_l = ($wake_msg, $msg);  
    return @out_msg_l;
}


sub trigger_action_sysinfo {
    my ($msg, $msg_hash, $session_id) = @_;
    $msg =~ s/<header>gosa_trigger_action_sysinfo<\/header>/<header>trigger_action_sysinfo<\/header>/;

    &main::change_fai_state('sysinfo', \@{$msg_hash->{macaddress}}, $session_id);
    my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
    if( defined $jobdb_id) {
        my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=$jobdb_id";
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7); 
        my $res = $main::job_db->exec_statement($sql_statement);
    }

    my @out_msg_l = ($msg);  
    return @out_msg_l;
}


sub new_key_for_client {
    my ($msg, $msg_hash, $session_id) = @_;

    my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
    if( defined $jobdb_id) {
        my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=$jobdb_id";
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7); 
        my $res = $main::job_db->exec_statement($sql_statement);
    }
    
    $msg =~ s/<header>gosa_new_key_for_client<\/header>/<header>new_key<\/header>/;
    my @out_msg_l = ($msg);  
    return @out_msg_l;
}


sub trigger_action_rescan {
    my ($msg, $msg_hash, $session_id) = @_;

    my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
    if( defined $jobdb_id) {
        my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=$jobdb_id";
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7); 
        my $res = $main::job_db->exec_statement($sql_statement);
    }


    $msg =~ s/<header>gosa_trigger_action_rescan<\/header>/<header>detect_hardware<header>/;
    my @out_msg_l = ($msg);  
    return @out_msg_l;
}


sub trigger_action_wake {
    my ($msg, $msg_hash, $session_id) = @_;
    
    my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
    if( defined $jobdb_id) {
        my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=$jobdb_id";
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7); 
        my $res = $main::job_db->exec_statement($sql_statement);
    }

    # build out message
    my $out_hash = &create_xml_hash("trigger_wake", "GOSA", "KNOWN_SERVER");
    foreach (@{$msg_hash->{'macaddress'}}) {
        &add_content2xml_hash($out_hash, 'macaddress', $_);
    }
    if (defined $jobdb_id){
        &add_content2xml_hash($out_hash, 'jobdb_id', $jobdb_id);
    }
    my $out_msg = &create_xml_string($out_hash);
    
    # invoke trigger wake for this gosa-si-server
    &main::server_server_com::trigger_wake($out_msg, $out_hash, $session_id);

    # send trigger wake to all other gosa-si-server
    my @out_msg_l = ($out_msg);  
    return @out_msg_l;
}


sub get_available_kernel {
  my ($msg, $msg_hash, $session_id) = @_;

  my $source = @{$msg_hash->{'source'}}[0];
  my $target = @{$msg_hash->{'target'}}[0];
  my $fai_release= @{$msg_hash->{'fai_release'}}[0];

  my @kernel;
  # Get Kernel packages for release
  my $sql_statement = "SELECT * FROM $main::packages_list_tn WHERE distribution='$fai_release' AND package LIKE 'linux\-image\-%'";
  my $res_hash = $main::packages_list_db->select_dbentry($sql_statement);
  my %data;
  my $i=1;

  foreach my $package (keys %{$res_hash}) {
    $data{"answer".$i++}= $data{"answer".$i++}= ${$res_hash}{$package}->{'package'};
  }
  $data{"answer".$i++}= "default";

  my $out_msg = &build_msg("get_available_kernel", $target, $source, \%data);
  my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
  if (defined $forward_to_gosa) {
    $out_msg =~s/<\/xml>/<forward_to_gosa>$forward_to_gosa<\/forward_to_gosa><\/xml>/;
  }

  return ( $out_msg );
}

sub trigger_activate_new {
	my ($msg, $msg_hash, $session_id) = @_;

	my $source = @{$msg_hash->{'source'}}[0];
	my $target = @{$msg_hash->{'target'}}[0];
	my $header= @{$msg_hash->{'header'}}[0];
	my $mac= (defined($msg_hash->{'mac'}))?@{$msg_hash->{'mac'}}[0]:undef;
	my $ogroup= (defined($msg_hash->{'ogroup'}))?@{$msg_hash->{'ogroup'}}[0]:undef;
	my $timestamp= (defined($msg_hash->{'timestamp'}))?@{$msg_hash->{'timestamp'}}[0]:undef;
	my $base= (defined($msg_hash->{'base'}))?@{$msg_hash->{'base'}}[0]:undef;
	my $hostname= (defined($msg_hash->{'fqdn'}))?@{$msg_hash->{'fqdn'}}[0]:undef;
	my $ip_address= (defined($msg_hash->{'ip'}))?@{$msg_hash->{'ip'}}[0]:undef;
	my $dhcp_statement= (defined($msg_hash->{'dhcp'}))?@{$msg_hash->{'dhcp'}}[0]:undef;
	my $jobdb_id= (defined($msg_hash->{'jobdb_id'}))?@{$msg_hash->{'jobdb_id'}}[0]:undef;

    # Sanity check for base
    if (ref($base) eq "HASH") {
        # Incoming msg has a xml tag 'base' but no content
        $base = undef;
    }

    # In case that the client is sleeping, wake it up
    my %data = ( 'macaddress'  => $mac );
    my $wake_msg = &build_msg("trigger_wake", "GOSA", "KNOWN_SERVER", \%data);
    &main::server_server_com::trigger_wake($msg, $msg_hash, $session_id);
    my $sql_statement= "SELECT * FROM $main::known_server_tn";
    my $query_res = $main::known_server_db->select_dbentry( $sql_statement ); 
    while( my ($hit_num, $hit) = each %{ $query_res } ) {    
        my $host_name = $hit->{hostname};
        my $host_key = $hit->{hostkey};
        $wake_msg =~ s/<target>\S+<\/target>/<target>$host_name<\/target>/g;
        my $error = &main::send_msg_to_target($wake_msg, $host_name, $host_key, $header, $session_id);
    }

	my $ldap_handle = &main::get_ldap_handle();
	my $ldap_entry;
	my $ogroup_entry;
	my $changed_attributes_counter = 0;

    my $activate_client = 0;
	
    if(defined($ogroup)) {
      my $ldap_mesg= $ldap_handle->search(
        base => $main::ldap_base,
        scope => 'sub',
        filter => "(&(objectClass=gosaGroupOfnames)(cn=$ogroup))",
      );
      if($ldap_mesg->count == 1) {
        $ogroup_entry= $ldap_mesg->pop_entry();
        &main::daemon_log("$session_id DEBUG: A GosaGroupOfNames with cn '$ogroup' was found in base '".$main::ldap_base."'!", 5);
      } elsif ($ldap_mesg->count == 0) {
        &main::daemon_log("$session_id ERROR: A GosaGroupOfNames with cn '$ogroup' was not found in base '".$main::ldap_base."'!", 1);
        $main::job_db->exec_statement("UPDATE ".$main::job_queue_tn." SET status = 'waiting', timestamp = '".(&calc_timestamp(&get_time(), 'plus', 60))."' WHERE id = $jobdb_id");
        return undef;
      } else {
        &main::daemon_log("$session_id ERROR: More than one ObjectGroups with cn '$ogroup' was found in base '".$main::ldap_base."'!", 1);
        $main::job_db->exec_statement("UPDATE ".$main::job_queue_tn." SET status = 'waiting', timestamp = '".(&calc_timestamp(&get_time(), 'plus', 60))."' WHERE id = $jobdb_id");
        return undef;
      }

      # build the base, use optional base parameter or take it from ogroup
      if(!(defined($base) && (length($base) > 0))) {
        # Subtract the ObjectGroup cn
        $base = $1 if $ogroup_entry->dn =~ /cn=$ogroup,ou=groups,(.*)$/;
        &main::daemon_log("$session_id DEBUG: New base for system with mac address '$mac' is '$base'", 5);
      }
    }

    # prepend ou=systems (configurable through config)
    $base = $main::new_systems_ou.",".$base;

    # Search for an existing entry (should be in ou=incoming)
    my $ldap_mesg= $ldap_handle->search(
      base => $main::ldap_base,
      scope => 'sub',
      filter => "(&(objectClass=GOhard)(|(macAddress=$mac)(dhcpHWaddress=$mac)))",
    );

    # TODO: Find a way to guess an ip address for hosts with no ldap entry (MAC->ARP->IP)
    if($ldap_mesg->count == 1) {
      &main::daemon_log("$session_id DEBUG: One system with mac address '$mac' was found in base '".$main::ldap_base."'!", 5);
      # Get the entry from LDAP
      $ldap_entry= $ldap_mesg->pop_entry();

      if(!($ldap_entry->dn() eq "cn=".$ldap_entry->get_value('cn').",$base")) {
        # Move the entry to the new ou
        $ldap_entry->changetype('moddn');
        $ldap_entry->add(
          newrdn => "cn=".$ldap_entry->get_value('cn'),
          deleteoldrdn => 1,
          newsuperior => $base,
        );
        # To prevent replication problems just re-queue the job with 10 seconds in the future
        my $moddn_result = $ldap_entry->update($ldap_handle);
        if ($moddn_result->code() != 0) {
            my $error_string = "Moving the system with mac address '$mac' to new base '$base' failed (code '".$moddn_result->code()."') with '".$moddn_result->{'errorMessage'}."'!";
            &main::daemon_log("$session_id ERROR: $error_string", 1);
            my $sql = "UPDATE $main::job_queue_tn SET status='error', result='$error_string' WHERE id=$jobdb_id";
            return undef;
        } else {
          &main::daemon_log("$session_id INFO: System with mac address '$mac' was moved to base '".$main::ldap_base."'! Re-queuing job.", 4);
          $main::job_db->exec_statement("UPDATE ".$main::job_queue_tn." SET status = 'waiting', timestamp = '".(&calc_timestamp(&get_time(), 'plus', 10))."' WHERE id = $jobdb_id");
          return undef;
        }
      }

    } elsif ($ldap_mesg->count == 0) {
      &main::daemon_log("$session_id WARNING: No System with mac address '$mac' was found in base '".$main::ldap_base."'! Re-queuing job.", 4);
      my $sql_statement = "UPDATE ".$main::job_queue_tn.
              " SET status='waiting', timestamp = '".(&calc_timestamp(&get_time(), 'plus', 60))."' ".
              " WHERE id = $jobdb_id";
      $main::job_db->exec_statement($sql_statement);
      return undef;
    }

    $ldap_mesg= $ldap_handle->search(
      base => $main::ldap_base,
      scope => 'sub',
      filter => "(&(objectClass=GOhard)(|(macAddress=$mac)(dhcpHWaddress=$mac)))",
    );

    # TODO: Find a way to guess an ip address for hosts with no ldap entry (MAC->ARP->IP)
    if($ldap_mesg->count == 1) {
      $ldap_entry= $ldap_mesg->pop_entry();
      # Check for needed objectClasses
      my $oclasses = $ldap_entry->get_value('objectClass', asref => 1);
      foreach my $oclass ("FAIobject", "GOhard", "gotoWorkstation") {
        if(!(scalar grep $_ eq $oclass, map {$_ => 1} @$oclasses)) {
          &main::daemon_log("$session_id INFO: Adding objectClass '$oclass' to system entry with mac adress '$mac'", 1);
          $ldap_entry->add(
            objectClass => $oclass,
          );
          my $oclass_result = $ldap_entry->update($ldap_handle);
          if ($oclass_result->code() != 0) {
            &main::daemon_log("$session_id ERROR: Adding the ObjectClass '$oclass' failed (code '".$oclass_result->code()."') with '".$oclass_result->{'errorMessage'}."'!", 1);
          } else {
            &main::daemon_log("$session_id DEBUG: Adding the ObjectClass '$oclass' to '".($ldap_entry->dn())."' succeeded!", 5);
          }
        }
      }

      # Set FAIstate
      if(defined($ldap_entry->get_value('FAIstate'))) {
        if(!($ldap_entry->get_value('FAIstate') eq 'install')) {
          $ldap_entry->replace(
            'FAIstate' => 'install'
          );
          my $replace_result = $ldap_entry->update($ldap_handle);
          if ($replace_result->code() != 0) {
            &main::daemon_log("$session_id ERROR: Setting the FAIstate to install failed with code '".$replace_result->code()."') and message '".$replace_result->{'errorMessage'}."'!", 1);
          } else {
            &main::daemon_log("$session_id DEBUG: Setting the FAIstate to install for '".($ldap_entry->dn())."' succeeded!", 5);
          }
        }
      } else {
        $ldap_entry->add(
          'FAIstate' => 'install'
        );
        my $add_result = $ldap_entry->update($ldap_handle);
        if ($add_result->code() != 0) {
          &main::daemon_log("$session_id ERROR: Setting the FAIstate to install failed with code '".$add_result->code()."') and message '".$add_result->{'errorMessage'}."'!", 1);
        } else {
          &main::daemon_log("$session_id DEBUG: Setting the FAIstate to install for '".($ldap_entry->dn())."' succeeded!", 5);
        }
      }


    } elsif ($ldap_mesg->count == 0) {
      # TODO: Create a new entry
      # $ldap_entry = Net::LDAP::Entry->new();
      # $ldap_entry->dn("cn=$mac,$base");
      &main::daemon_log("$session_id WARNING: No System with mac address '$mac' was found in base '".$main::ldap_base."'! Re-queuing job.", 4);
      $main::job_db->exec_statement("UPDATE ".$main::job_queue_tn." SET status = 'waiting', timestamp = '".(&calc_timestamp(&get_time(), 'plus', 60))."' WHERE id = $jobdb_id");
      return undef;
    } else {
      &main::daemon_log("$session_id ERROR: More than one system with mac address '$mac' was found in base '".$main::ldap_base."'!", 1);
    }

    # Add to ObjectGroup
    my $ogroup_member = $ogroup_entry->get_value('member', asref => 1);
    if( (!defined($ogroup_member)) ||
        (!defined($ldap_entry)) ||
        (!defined($ldap_entry->dn)) ||
        (!(scalar grep $_ eq $ldap_entry->dn, @{$ogroup_member}))) {
      $ogroup_entry->add (
        'member' => $ldap_entry->dn(),
      );
      my $ogroup_result = $ogroup_entry->update($ldap_handle);
      if ($ogroup_result->code() != 0) {
        &main::daemon_log("$session_id ERROR: Updating the ObjectGroup '$ogroup' failed (code '".$ogroup_result->code()."') with '".$ogroup_result->{'errorMessage'}."'!", 1);
      } else {
        &main::daemon_log("$session_id DEBUG: Updating the ObjectGroup '$ogroup' for member '".($ldap_entry->dn())."' succeeded!", 5);
      }
    } else {
      &main::daemon_log("$session_id DEBUG: System with mac address '$mac' is already a member of ObjectGroup '$ogroup'.", 5);
    }

    # Finally set gotoMode to active
    if(defined($ldap_entry->get_value('gotoMode'))) {
      if(!($ldap_entry->get_value('gotoMode') eq 'active')) {
        $ldap_entry->replace(
          'gotoMode' => 'active'
        );
        my $activate_result = $ldap_entry->update($ldap_handle);
        if ($activate_result->code() != 0) {
          &main::daemon_log("$session_id ERROR: Activating system '".$ldap_entry->dn()."' failed (code '".$activate_result->code()."') with '".$activate_result->{'errorMessage'}."'!", 1);
        } else {
          &main::daemon_log("$session_id DEBUG: Activating system '".$ldap_entry->dn()."' succeeded!", 5);
          $activate_client = 1;
        }
      } else {
          $activate_client = 1;
      }
    } else {
      $ldap_entry->add(
        'gotoMode' => 'active'
      );
      my $activate_result = $ldap_entry->update($ldap_handle);
      if ($activate_result->code() != 0) {
        &main::daemon_log("$session_id ERROR: Activating system '".$ldap_entry->dn()."' failed (code '".$activate_result->code()."') with '".$activate_result->{'errorMessage'}."'!", 1);
      } else {
        &main::daemon_log("$session_id DEBUG: Activating system '".$ldap_entry->dn()."' succeeded!", 5);
        $activate_client = 1;
      }
    }

    if($activate_client == 1) {
        &main::daemon_log("$session_id DEBIG: Activating system with mac address '$mac'!", 5);

        # Create delivery list
        my @out_msg_l;

        # Set job to done
        $main::job_db->exec_statement("UPDATE jobs SET status = 'done' WHERE id = $jobdb_id");

        # create set_activated_for_installation message for delivery
        my $out_hash = &create_xml_hash("set_activated_for_installation", $source, $target);
        my $out_msg = &create_xml_string($out_hash);
        push(@out_msg_l, $out_msg);

        # Return delivery list of messages
        return @out_msg_l;

    } else {
      &main::daemon_log("$session_id WARNING: Activating system with mac address '$mac' failed! Re-queuing job.", 4);
      $main::job_db->exec_statement("UPDATE ".$main::job_queue_tn." SET status = 'waiting',  timestamp = '".(&calc_timestamp(&get_time(), 'plus', 60))."' WHERE id = $jobdb_id");
    }
    return undef;
}


#sub get_dak_keyring {
#    my ($msg, $msg_hash) = @_;
#    my $source = @{$msg_hash->{'source'}}[0];
#    my $target = @{$msg_hash->{'target'}}[0];
#    my $header= @{$msg_hash->{'header'}}[0];
#    my $session_id = @{$msg_hash->{'session_id'}}[0];
#
#    # build return message with twisted target and source
#    my $out_hash = &main::create_xml_hash("answer_$header", $target, $source);
#    &add_content2xml_hash($out_hash, "session_id", $session_id);
#
#    my @keys;
#    my %data;
#
#    my $keyring = $main::dak_signing_keys_directory."/keyring.gpg";
#
#    my $gpg_cmd = `which gpg`; chomp $gpg_cmd;
#    my $gpg     = "$gpg_cmd --no-default-keyring --no-random-seed --keyring $keyring";
#
#    # Check if the keyrings are in place and readable
#    if(
#        &run_as($main::dak_user, "test -r $keyring")->{'resultCode'} != 0
#    ) {
#        &add_content2xml_hash($out_hash, "error", "DAK Keyring is not readable");
#    } else {
#        my $command = "$gpg --list-keys";
#        my $output = &run_as($main::dak_user, $command);
#        &main::daemon_log("$session_id DEBUG: ".$output->{'command'}, 7);
#
#        my $i=0;
#        foreach (@{$output->{'output'}}) {
#            if ($_ =~ m/^pub\s.*$/) {
#                ($keys[$i]->{'pub'}->{'length'}, $keys[$i]->{'pub'}->{'uid'}, $keys[$i]->{'pub'}->{'created'}) = ($1, $2, $3)
#                if $_ =~ m/^pub\s*?(\w*?)\/(\w*?)\s(\d{4}-\d{2}-\d{2})/;
#                $keys[$i]->{'pub'}->{'expires'} = $1 if $_ =~ m/^pub\s*?\w*?\/\w*?\s\d{4}-\d{2}-\d{2}\s\[expires:\s(\d{4}-\d{2}-\d{2})\]/;
#                $keys[$i]->{'pub'}->{'expired'} = $1 if $_ =~ m/^pub\s*?\w*?\/\w*?\s\d{4}-\d{2}-\d{2}\s\[expired:\s(\d{4}-\d{2}-\d{2})\]/;
#            } elsif ($_ =~ m/^sub\s.*$/) {
#                ($keys[$i]->{'sub'}->{'length'}, $keys[$i]->{'sub'}->{'uid'}, $keys[$i]->{'sub'}->{'created'}) = ($1, $2, $3)
#                if $_ =~ m/^sub\s*?(\w*?)\/(\w*?)\s(\d{4}-\d{2}-\d{2})/;
#                $keys[$i]->{'sub'}->{'expires'} = $1 if $_ =~ m/^pub\s*?\w*?\/\w*?\s\d{4}-\d{2}-\d{2}\s\[expires:\s(\d{4}-\d{2}-\d{2})\]/;
#                $keys[$i]->{'sub'}->{'expired'} = $1 if $_ =~ m/^pub\s*?\w*?\/\w*?\s\d{4}-\d{2}-\d{2}\s\[expired:\s(\d{4}-\d{2}-\d{2})\]/;
#            } elsif ($_ =~ m/^uid\s.*$/) {
#                push @{$keys[$i]->{'uid'}}, $1 if $_ =~ m/^uid\s*?([^\s].*?)$/;
#            } elsif ($_ =~ m/^$/) {
#                $i++;
#            }
#        }
#    }
#
#    my $i=0;
#    foreach my $key (@keys) {
#        #    &main::daemon_log(Dumper($key));
#        &add_content2xml_hash($out_hash, "answer".$i++, $key);
#    }
#    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
#    if (defined $forward_to_gosa) {
#        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
#    }
#    return &create_xml_string($out_hash);
#}
#
#
#sub import_dak_key {
#    my ($msg, $msg_hash) = @_;
#    my $source = @{$msg_hash->{'source'}}[0];
#    my $target = @{$msg_hash->{'target'}}[0];
#    my $header= @{$msg_hash->{'header'}}[0];
#    my $session_id = @{$msg_hash->{'session_id'}}[0];
#    my $key = &decode_base64(@{$msg_hash->{'key'}}[0]);
#
#    # build return message with twisted target and source
#    my $out_hash = &main::create_xml_hash("answer_$header", $target, $source);
#    &add_content2xml_hash($out_hash, "session_id", $session_id);
#
#    my %data;
#
#    my $keyring = $main::dak_signing_keys_directory."/keyring.gpg";
#
#    my $gpg_cmd = `which gpg`; chomp $gpg_cmd;
#    my $gpg     = "$gpg_cmd --no-default-keyring --no-random-seed --keyring $keyring";
#
#    # Check if the keyrings are in place and writable
#    if(
#        &run_as($main::dak_user, "test -w $keyring")->{'resultCode'} != 0
#    ) {
#        &add_content2xml_hash($out_hash, "error", "DAK Keyring is not writable");
#    } else {
#        my $keyfile;
#        open($keyfile, ">/tmp/gosa_si_tmp_dak_key");
#        print $keyfile $key;
#        close($keyfile);
#        my $command = "$gpg --import /tmp/gosa_si_tmp_dak_key";
#        my $output = &run_as($main::dak_user, $command);
#        &main::daemon_log("$session_id DEBUG: ".$output->{'command'}, 7);
#        unlink("/tmp/gosa_si_tmp_dak_key");
#
#        if($output->{'resultCode'} != 0) {
#            &add_content2xml_hash($out_hash, "error", "Import of DAK key failed! Output was '".$output->{'output'}."'");
#        } else {
#            &add_content2xml_hash($out_hash, "answer", "Import of DAK key successfull! Output was '".$output->{'output'}."'");
#        }
#    }
#
#    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
#    if (defined $forward_to_gosa) {
#        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
#    }
#    return &create_xml_string($out_hash);
#}
#
#
#sub remove_dak_key {
#    my ($msg, $msg_hash) = @_;
#    my $source = @{$msg_hash->{'source'}}[0];
#    my $target = @{$msg_hash->{'target'}}[0];
#    my $header= @{$msg_hash->{'header'}}[0];
#    my $session_id = @{$msg_hash->{'session_id'}}[0];
#    my $key = @{$msg_hash->{'keyid'}}[0];
#    # build return message with twisted target and source
#    my $out_hash = &main::create_xml_hash("answer_$header", $target, $source);
#    &add_content2xml_hash($out_hash, "session_id", $session_id);
#
#    my %data;
#
#    my $keyring = $main::dak_signing_keys_directory."/keyring.gpg";
#
#    my $gpg_cmd = `which gpg`; chomp $gpg_cmd;
#    my $gpg     = "$gpg_cmd --no-default-keyring --no-random-seed --homedir ".$main::dak_signing_keys_directory." --keyring $keyring";
#
#    # Check if the keyrings are in place and writable
#    if(
#        &run_as($main::dak_user, "test -w $keyring")->{'resultCode'} != 0
#    ) {
#        &add_content2xml_hash($out_hash, "error", "DAK keyring is not writable");
#    } else {
#        # Check if the key is present in the keyring
#        if(&run_as($main::dak_user, "$gpg --list-keys $key")->{'resultCode'} == 0) {
#            my $command = "$gpg --batch --yes --delete-key $key";
#            my $output = &run_as($main::dak_user, $command);
#            &main::daemon_log("$session_id DEBUG: ".$output->{'command'}, 7);
#        } else {
#            &add_content2xml_hash($out_hash, "error", "DAK key with id '$key' was not found in keyring");
#        }
#    }
#
#    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
#    if (defined $forward_to_gosa) {
#        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
#    }
#    return &create_xml_string($out_hash);
#}


#sub get_dak_queue {
#    my ($msg, $msg_hash, $session_id) = @_;
#    my %data;
#    my $source = @{$msg_hash->{'source'}}[0];
#    my $target = @{$msg_hash->{'target'}}[0];
#    my $header= @{$msg_hash->{'header'}}[0];
#
#    my %data;
#
#    foreach my $dir ("unchecked", "new", "accepted") {
#        foreach my $file(<"$main::dak_queue_directory/$dir/*.changes">) {
#        }
#    }
#
#    my $out_msg = &build_msg("get_dak_queue", $target, $source, \%data);
#    my @out_msg_l = ($out_msg);
#    return @out_msg_l;
#}

## @method get_hosts_with_module
# Reports all GOsa-si-server providing the given module. 
# @param msg - STRING - xml message with tag get_hosts_with_module
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub get_hosts_with_module {
    my ($msg, $msg_hash, $session_id) = @_;
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $header= @{$msg_hash->{'header'}}[0];
    my $module_name = @{$msg_hash->{'module_name'}}[0];
    my $out_hash = &create_xml_hash($header, $target, $source);

    # Sanity check of module_name
    if ((not exists $msg_hash->{'module_name'}) || (@{$msg_hash->{'module_name'}} != 1))  {
        &add_content2xml_hash($out_hash, "error_string", "no module_name specified or module_name tag invalid");
        &add_content2xml_hash($out_hash, "error", "module_name");
        &main::daemon_log("$session_id ERROR: no module_name specified or module_name tag invalid: $msg", 1); 
        return (&create_xml_string($out_hash));
    }

    my $out_msg = &create_xml_string($out_hash);

    # Check localhost for module_name
    if (exists @{$main::known_modules->{'GosaPackages'}}[2]->{$module_name}) {
        my ($local_ip, $local_port) = split(/:/, $target);
        my $network_interface= &get_interface_for_ip($local_ip);
        my $local_mac = &get_mac_for_interface($network_interface);
        $out_msg =~ s/<\/xml>/<result>host0<\/result> <\/xml>/;
        my $host_infos = "<ip>$local_ip</ip>";
        $host_infos .= " <mac>$local_mac</mac>"; 
        $out_msg =~  s/<\/xml>/\n<answer0> $host_infos <\/answer0> \n <\/xml>/;
    }

    # Search for opsi hosts in server_db
    my $sql = "SELECT * FROM $main::known_server_tn WHERE loaded_modules LIKE '%$module_name%'"; 
    my $res = $main::known_server_db->select_dbentry($sql);
    while (my ($hit_id, $hit_hash) = each %$res) {
        $out_msg =~ s/<\/xml>/<result>host$hit_id<\/result> <\/xml>/;
        my $host_infos = "<ip>".$hit_hash->{'hostname'}."</ip>";
        $host_infos .= " <mac>".$hit_hash->{'macaddress'}."</mac>"; 
        $out_msg =~  s/<\/xml>/\n<answer$hit_id> $host_infos <\/answer$hit_id> \n <\/xml>/;
    }

    return $out_msg;
}

# vim:ts=4:shiftwidth:expandtab
1;