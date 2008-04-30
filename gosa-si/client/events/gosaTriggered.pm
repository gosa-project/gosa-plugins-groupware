package gosaTriggered;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    "get_events",
    "usr_msg",
    "trigger_action_localboot",
    "trigger_action_halt",
    "trigger_action_faireboot",
    "trigger_action_reboot",
#    "trigger_action_memcheck",   # not yet implemented
    "trigger_action_reinstall",
    "trigger_action_update",
    "trigger_action_instant_update",
#    "trigger_action_sysinfo",    # not yet implemented
    );
@EXPORT = @events;

use strict;
use warnings;
use GOSA::GosaSupportDaemon;
use Data::Dumper;
use MIME::Base64;

BEGIN {}

END {}


sub get_events { return \@events; }

sub usr_msg {
    my ($msg, $msg_hash) = @_;


    my $to = @{$msg_hash->{'usr'}}[0];
    my $subject = &decode_base64(@{$msg_hash->{'subject'}}[0]);
    my $message = &decode_base64(@{$msg_hash->{'message'}}[0]);
    system( "/usr/bin/goto-notify user-message '$to' '$subject' '$message'" );

    # give gosa-si-server feedback, that msg was received
    $msg =~ s/<header>usr_msg<\/header>/<header>confirm_usr_msg<\/header>/g;
    return $msg;
}


sub trigger_action_localboot {
    my ($msg, $msg_hash) = @_;
    my $timeout;

    if((not exists $msg_hash->{timeout} ) || (1 != @{$msg_hash->{timeout}} ) ) {
        $timeout = -1;
    } 
    else {
        $timeout = @{$msg_hash->{timeout}}[0];
    }

    # check logged in user
    my $logged_in_user = 1;
    if( $logged_in_user ) {
        # TODO do something
    }
    else {
        $timeout = 0;
    }
        
    # execute function
    if( $timeout == 0 ) {
        print STDERR ("shutdown -r +$timeout\n");
    }
    elsif( $timeout > 0 ) {
        print STDERR ("shutdown -r +$timeout\n");
    }
    elsif( $timeout < 0 ) {
        print STDERR "The administrator has sent a signal to reboot this workstation. It will reboot after you've logged out.\n";
        open(FILE, "> /etc/gosa-si/event");
        print FILE "trigger_action_localboot\n";
        close(FILE);
    }
    else {
        # TODO do something, error handling, logging
    }

    return;
}


sub trigger_action_faireboot {
    my ($msg, $msg_hash) = @_;
	&main::daemon_log("DEBUG: run /usr/sbin/faireboot\n", 7); 
    system("/usr/sbin/faireboot");
    return;
}


sub trigger_action_reboot {
    my ($msg, $msg_hash) = @_;
    my $timeout;

    if((not exists $msg_hash->{timeout} ) || (1 != @{$msg_hash->{timeout}} ) ) {
        $timeout = 0;
    } 
    else {
        $timeout = @{$msg_hash->{timeout}}[0];
    }

    # check logged in user
    my @user_list = &get_logged_in_users;
    if( @user_list >= 1 ) {
    	system( "/usr/bin/goto-notify reboot" );
        open(FILE, "> /etc/gosa-si/event");
        print FILE "reboot\n";
        close(FILE);
    }
    else {
    	system( "/sbin/shutdown -r +$timeout &" );
    }

    return;
}


sub trigger_action_halt {
    my ($msg, $msg_hash) = @_;
    my $timeout;

    if((not exists $msg_hash->{timeout} ) || (1 != @{$msg_hash->{timeout}} ) ) {
        $timeout = 0;
    } 
    else {
        $timeout = @{$msg_hash->{timeout}}[0];
    }

    # check logged in user
    my @user_list = &get_logged_in_users;
    if( @user_list >= 1 ) {
    	system( "/usr/bin/goto-notify halt" );
        open(FILE, "> /etc/gosa-si/event");
        print FILE "halt\n";
        close(FILE);
    }
    else {
    	system( "/sbin/shutdown -h +$timeout &" );
    }

    return;
}


sub trigger_action_reinstall {
    my ($msg, $msg_hash) = @_;

    # check logged in user
    my @user_list = &get_logged_in_users;
    if( @user_list >= 1 ) {
    	system( "/usr/bin/goto-notify install" );
        open(FILE, "> /etc/gosa-si/event");
        print FILE "install\n";
        close(FILE);
    }
    else {
    	system( "/sbin/shutdown -r now &" );
    }

    return;
}


# Backward compatibility
sub trigger_action_update {
    my ($msg, $msg_hash) = @_;

    # Execute update
    system( "DEBIAN_FRONTEND=noninteractive /usr/sbin/fai-softupdate &" );

    return;
}

# Backward compatibility
sub trigger_action_instant_update {
    my ($msg, $msg_hash) = @_;

    # Execute update
    system( "DEBIAN_FRONTEND=noninteractive /usr/sbin/fai-softupdate &" );

    return;
}


1;
