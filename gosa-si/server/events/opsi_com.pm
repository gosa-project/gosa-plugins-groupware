## @file
# @details A GOsa-SI-server event module containing all functions for message handling.
# @brief Implementation of an event module for GOsa-SI-server. 


package opsi_com;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    "get_events",
    "opsi_install_client",
    "opsi_get_netboot_products",  
    "opsi_get_local_products",
    "opsi_get_client_hardware",
    "opsi_get_client_software",
    "opsi_get_product_properties",
    "opsi_set_product_properties",
    "opsi_list_clients",
    "opsi_del_client",
    "opsi_add_client",
    "opsi_modify_client",
    "opsi_add_product_to_client",
    "opsi_del_product_from_client",
	"opsi_createLicensePool",
	"opsi_deleteLicensePool",
	"opsi_createLicense",
	"opsi_assignSoftwareLicenseToHost",
	"opsi_unassignSoftwareLicenseFromHost",
	"opsi_unassignAllSoftwareLicensesFromHost",
	"opsi_getSoftwareLicense_hash",
	"opsi_getLicensePool_hash",
	"opsi_getSoftwareLicenseUsages_listOfHashes",
	"opsi_getLicensePools_listOfHashes",
	"opsi_getLicenseInformationForProduct",
	"opsi_getPool",
	"opsi_removeLicense",
	"opsi_test",
   );
@EXPORT = @events;

use strict;
use warnings;
use GOSA::GosaSupportDaemon;
use Data::Dumper;
use XML::Quote qw(:all);

BEGIN {}

END {}

# ----------------------------------------------------------------------------
#                          D E C L A R A T I O N S
# ----------------------------------------------------------------------------

my $licenseTyp_hash = { 'OEM'=>'', 'VOLUME'=>'', 'RETAIL'=>''};



# ----------------------------------------------------------------------------
#                            S U B R O U T I N E S
# ----------------------------------------------------------------------------


################################
#
# @brief A function returning a list of functions which are exported by importing the module.
# @return List of all provided functions
#
sub get_events {
    return \@events;
}

################################
#
# @brief Checks if there is a specified tag and if the the tag has a content.
# @return 0|1
#
sub _check_xml_tag_is_ok {
	my ($msg_hash,$tag) = @_;
	if (not defined $msg_hash->{$tag}) {
		$_ = "message contains no tag '$tag'";
		return 0;
	}
	if (ref @{$msg_hash->{$tag}}[0] eq 'HASH') {
		$_ = "message tag '$tag' has no content";
		return  0;
	}
	return 1;
}

################################
#
# @brief Writes the log line and returns the error message for GOsa.
#
sub _give_feedback {
	my ($msg, $msg_hash, $session_id, $error) = @_;
	&main::daemon_log("$session_id ERROR: $error: ".$msg, 1);
	my $out_hash = &main::create_xml_hash("error", $main::server_address, @{$msg_hash->{'source'}}[0], $error);
	return &create_xml_string($out_hash);
}

## @method opsi_add_product_to_client
# Adds an Opsi product to an Opsi client.
# @param msg - STRING - xml message with tags hostId and productId
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_add_product_to_client {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my ($hostId, $productId);
    my $error = 0;

    # Build return message
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

    # Sanity check of needed parameter
    if ((not exists $msg_hash->{'hostId'}) || (@{$msg_hash->{'hostId'}} != 1) || (@{$msg_hash->{'hostId'}}[0] eq ref 'HASH'))  {
        $error++;
        &add_content2xml_hash($out_hash, "error_string", "no hostId specified or hostId tag invalid");
        &add_content2xml_hash($out_hash, "error", "hostId");
        &main::daemon_log("$session_id ERROR: no hostId specified or hostId tag invalid: $msg", 1); 

    }
    if ((not exists $msg_hash->{'productId'}) || (@{$msg_hash->{'productId'}} != 1) || (@{$msg_hash->{'productId'}}[0] eq ref 'HASH')) {
        $error++;
        &add_content2xml_hash($out_hash, "error_string", "no productId specified or productId tag invalid");
        &add_content2xml_hash($out_hash, "error", "productId");
        &main::daemon_log("$session_id ERROR: no productId specified or procutId tag invalid: $msg", 1); 
    }

    if (not $error) {
        # Get hostId
        $hostId = @{$msg_hash->{'hostId'}}[0];
        &add_content2xml_hash($out_hash, "hostId", $hostId);

        # Get productID
        $productId = @{$msg_hash->{'productId'}}[0];
        &add_content2xml_hash($out_hash, "productId", $productId);

        # Do an action request for all these -> "setup".
        my $callobj = {
            method  => 'setProductActionRequest',
            params  => [ $productId, $hostId, "setup" ],
            id  => 1, }; 

        my $sres = $main::opsi_client->call($main::opsi_url, $callobj);
        my ($sres_err, $sres_err_string) = &check_opsi_res($sres);
        if ($sres_err){
            &main::daemon_log("$session_id ERROR: cannot add product: ".$sres_err_string, 1);
            &add_content2xml_hash($out_hash, "error", $sres_err_string);
        }
    } 

    # return message
    return ( &create_xml_string($out_hash) );
}

## @method opsi_del_product_from_client
# Deletes an Opsi-product from an Opsi-client. 
# @param msg - STRING - xml message with tags hostId and productId
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_del_product_from_client {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my ($hostId, $productId);
    my $error = 0;
    my ($sres, $sres_err, $sres_err_string);

    # Build return message
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

    # Sanity check of needed parameter
    if ((not exists $msg_hash->{'hostId'}) || (@{$msg_hash->{'hostId'}} != 1) || (@{$msg_hash->{'hostId'}}[0] eq ref 'HASH'))  {
        $error++;
        &add_content2xml_hash($out_hash, "error_string", "no hostId specified or hostId tag invalid");
        &add_content2xml_hash($out_hash, "error", "hostId");
        &main::daemon_log("$session_id ERROR: no hostId specified or hostId tag invalid: $msg", 1); 

    }
    if ((not exists $msg_hash->{'productId'}) || (@{$msg_hash->{'productId'}} != 1) || (@{$msg_hash->{'productId'}}[0] eq ref 'HASH')) {
        $error++;
        &add_content2xml_hash($out_hash, "error_string", "no productId specified or productId tag invalid");
        &add_content2xml_hash($out_hash, "error", "productId");
        &main::daemon_log("$session_id ERROR: no productId specified or procutId tag invalid: $msg", 1); 
    }

    # All parameter available
    if (not $error) {
        # Get hostId
        $hostId = @{$msg_hash->{'hostId'}}[0];
        &add_content2xml_hash($out_hash, "hostId", $hostId);

        # Get productID
        $productId = @{$msg_hash->{'productId'}}[0];
        &add_content2xml_hash($out_hash, "productId", $productId);


# : check the results for more than one entry which is currently installed
        #$callobj = {
        #    method  => 'getProductDependencies_listOfHashes',
        #    params  => [ $productId ],
        #    id  => 1, };
        #
        #my $sres = $main::opsi_client->call($main::opsi_url, $callobj);
        #my ($sres_err, $sres_err_string) = &check_opsi_res($sres);
        #if ($sres_err){
        #  &main::daemon_log("ERROR: cannot perform dependency check: ".$sres_err_string, 1);
        #  &add_content2xml_hash($out_hash, "error", $sres_err_string);
        #  return ( &create_xml_string($out_hash) );
        #}


        # Check to get product action list 
        my $callobj = {
            method  => 'getPossibleProductActions_list',
            params  => [ $productId ],
            id  => 1, };
        $sres = $main::opsi_client->call($main::opsi_url, $callobj);
        ($sres_err, $sres_err_string) = &check_opsi_res($sres);
        if ($sres_err){
            &main::daemon_log("$session_id ERROR: cannot get product action list: ".$sres_err_string, 1);
            &add_content2xml_hash($out_hash, "error", $sres_err_string);
            $error++;
        }
    }

    # Check action uninstall of product
    if (not $error) {
        my $uninst_possible= 0;
        foreach my $r (@{$sres->result}) {
            if ($r eq 'uninstall') {
                $uninst_possible= 1;
            }
        }
        if (!$uninst_possible){
            &main::daemon_log("$session_id ERROR: cannot uninstall product '$productId', product do not has the action 'uninstall'", 1);
            &add_content2xml_hash($out_hash, "error", "cannot uninstall product '$productId', product do not has the action 'uninstall'");
            $error++;
        }
    }

    # Set product state to "none"
    # Do an action request for all these -> "setup".
    if (not $error) {
        my $callobj = {
            method  => 'setProductActionRequest',
            params  => [ $productId, $hostId, "none" ],
            id  => 1, 
        }; 
        $sres = $main::opsi_client->call($main::opsi_url, $callobj);
        ($sres_err, $sres_err_string) = &check_opsi_res($sres);
        if ($sres_err){
            &main::daemon_log("$session_id ERROR: cannot delete product: ".$sres_err_string, 1);
            &add_content2xml_hash($out_hash, "error", $sres_err_string);
        }
    }

    # Return message
    return ( &create_xml_string($out_hash) );
}

## @method opsi_add_client
# Adds an Opsi client to Opsi.
# @param msg - STRING - xml message with tags hostId and macaddress
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_add_client {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my ($hostId, $mac);
    my $error = 0;
    my ($sres, $sres_err, $sres_err_string);

    # Build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

    # Sanity check of needed parameter
    if ((not exists $msg_hash->{'hostId'}) || (@{$msg_hash->{'hostId'}} != 1) || (@{$msg_hash->{'hostId'}}[0] eq ref 'HASH'))  {
        $error++;
        &add_content2xml_hash($out_hash, "error_string", "no hostId specified or hostId tag invalid");
        &add_content2xml_hash($out_hash, "error", "hostId");
        &main::daemon_log("$session_id ERROR: no hostId specified or hostId tag invalid: $msg", 1); 
    }
    if ((not exists $msg_hash->{'macaddress'}) || (@{$msg_hash->{'macaddress'}} != 1) || (@{$msg_hash->{'macaddress'}}[0] eq ref 'HASH'))  {
        $error++;
        &add_content2xml_hash($out_hash, "error_string", "no macaddress specified or macaddress tag invalid");
        &add_content2xml_hash($out_hash, "error", "macaddress");
        &main::daemon_log("$session_id ERROR: no macaddress specified or macaddress tag invalid: $msg", 1); 
    }

    if (not $error) {
        # Get hostId
        $hostId = @{$msg_hash->{'hostId'}}[0];
        &add_content2xml_hash($out_hash, "hostId", $hostId);

        # Get macaddress
        $mac = @{$msg_hash->{'macaddress'}}[0];
        &add_content2xml_hash($out_hash, "macaddress", $mac);

        my $name= $hostId;
        $name=~ s/^([^.]+).*$/$1/;
        my $domain= $hostId;
        $domain=~ s/^[^.]+\.(.*)$/$1/;
        my ($description, $notes, $ip);

        if (defined @{$msg_hash->{'description'}}[0]){
            $description = @{$msg_hash->{'description'}}[0];
        }
        if (defined @{$msg_hash->{'notes'}}[0]){
            $notes = @{$msg_hash->{'notes'}}[0];
        }
        if (defined @{$msg_hash->{'ip'}}[0]){
            $ip = @{$msg_hash->{'ip'}}[0];
        }

        my $callobj;
        $callobj = {
            method  => 'createClient',
            params  => [ $name, $domain, $description, $notes, $ip, $mac ],
            id  => 1,
        };

        $sres = $main::opsi_client->call($main::opsi_url, $callobj);
        ($sres_err, $sres_err_string) = &check_opsi_res($sres);
        if ($sres_err){
            &main::daemon_log("$session_id ERROR: cannot create client: ".$sres_err_string, 1);
            &add_content2xml_hash($out_hash, "error", $sres_err_string);
        } else {
            &main::daemon_log("$session_id INFO: add opsi client '$hostId' with mac '$mac'", 5); 
        }
    }

    # Return message
    return ( &create_xml_string($out_hash) );
}

## @method opsi_modify_client
# Modifies the parameters description, mac or notes for an Opsi client if the corresponding message tags are given.
# @param msg - STRING - xml message with tag hostId and optional description, mac or notes
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message    
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_modify_client {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my $hostId;
    my $error = 0;
    my ($sres, $sres_err, $sres_err_string);

    # Build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

    # Sanity check of needed parameter
    if ((not exists $msg_hash->{'hostId'}) || (@{$msg_hash->{'hostId'}} != 1) || (@{$msg_hash->{'hostId'}}[0] eq ref 'HASH'))  {
        $error++;
        &add_content2xml_hash($out_hash, "error_string", "no hostId specified or hostId tag invalid");
        &add_content2xml_hash($out_hash, "error", "hostId");
        &main::daemon_log("$session_id ERROR: no hostId specified or hostId tag invalid: $msg", 1); 
    }

    if (not $error) {
        # Get hostId
        $hostId = @{$msg_hash->{'hostId'}}[0];
        &add_content2xml_hash($out_hash, "hostId", $hostId);
        my $name= $hostId;
        $name=~ s/^([^.]+).*$/$1/;
        my $domain= $hostId;
        $domain=~ s/^[^.]+(.*)$/$1/;

        # Modify description, notes or mac if defined
        my ($description, $notes, $mac);
        my $callobj;
        if ((exists $msg_hash->{'description'}) && (@{$msg_hash->{'description'}} == 1) ){
            $description = @{$msg_hash->{'description'}}[0];
            $callobj = {
                method  => 'setHostDescription',
                params  => [ $hostId, $description ],
                id  => 1,
            };
            my $sres = $main::opsi_client->call($main::opsi_url, $callobj);
            my ($sres_err, $sres_err_string) = &check_opsi_res($sres);
            if ($sres_err){
                &main::daemon_log("ERROR: cannot set description: ".$sres_err_string, 1);
                &add_content2xml_hash($out_hash, "error", $sres_err_string);
            }
        }
        if ((exists $msg_hash->{'notes'}) && (@{$msg_hash->{'notes'}} == 1)) {
            $notes = @{$msg_hash->{'notes'}}[0];
            $callobj = {
                method  => 'setHostNotes',
                params  => [ $hostId, $notes ],
                id  => 1,
            };
            my $sres = $main::opsi_client->call($main::opsi_url, $callobj);
            my ($sres_err, $sres_err_string) = &check_opsi_res($sres);
            if ($sres_err){
                &main::daemon_log("ERROR: cannot set notes: ".$sres_err_string, 1);
                &add_content2xml_hash($out_hash, "error", $sres_err_string);
            }
        }
        if ((exists $msg_hash->{'mac'}) && (@{$msg_hash->{'mac'}} == 1)){
            $mac = @{$msg_hash->{'mac'}}[0];
            $callobj = {
                method  => 'setMacAddress',
                params  => [ $hostId, $mac ],
                id  => 1,
            };
            my $sres = $main::opsi_client->call($main::opsi_url, $callobj);
            my ($sres_err, $sres_err_string) = &check_opsi_res($sres);
            if ($sres_err){
                &main::daemon_log("ERROR: cannot set mac address: ".$sres_err_string, 1);
                &add_content2xml_hash($out_hash, "error", $sres_err_string);
            }
        }
    }

    # Return message
    return ( &create_xml_string($out_hash) );
}

    
## @method opsi_get_netboot_products
# Get netboot products for specific host.
# @param msg - STRING - xml message with tag hostId
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_get_netboot_products {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my $hostId;
    my $xml_msg;

    # Build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

    # Get hostId if defined
    if ((exists $msg_hash->{'hostId'}) && (@{$msg_hash->{'hostId'}} == 1))  {
        $hostId = @{$msg_hash->{'hostId'}}[0];
        &add_content2xml_hash($out_hash, "hostId", $hostId);
    }

    &add_content2xml_hash($out_hash, "xxx", "");
    $xml_msg = &create_xml_string($out_hash);
    # For hosts, only return the products that are or get installed
    my $callobj;
    $callobj = {
        method  => 'getNetBootProductIds_list',
        params  => [ ],
        id  => 1,
    };
    &main::daemon_log("$session_id DEBUG: send callobj to opsi_client: ".&opsi_callobj2string($callobj), 7);
    &main::daemon_log("$session_id DEBUG: opsi_url $main::opsi_url", 7);
    &main::daemon_log("$session_id DEBUG: waiting for answer from opsi_client!", 7);
    my $res = $main::opsi_client->call($main::opsi_url, $callobj);
    &main::daemon_log("$session_id DEBUG: get answer from opsi_client", 7);
    my %r = ();
    for (@{$res->result}) { $r{$_} = 1 }

    if (not &check_opsi_res($res)){

        if (defined $hostId){

            $callobj = {
                method  => 'getProductStates_hash',
                params  => [ $hostId ],
                id  => 1,
            };

            my $hres = $main::opsi_client->call($main::opsi_url, $callobj);
            if (not &check_opsi_res($hres)){
                my $htmp= $hres->result->{$hostId};

                # check state != not_installed or action == setup -> load and add
                foreach my $product (@{$htmp}){

                    if (!defined ($r{$product->{'productId'}})){
                        next;
                    }

                    # Now we've a couple of hashes...
                    if ($product->{'installationStatus'} ne "not_installed" or
                            $product->{'actionRequest'} eq "setup"){
                        my $state= "<state>".$product->{'installationStatus'}."</state><action>".$product->{'actionRequest'}."</action>";

                        $callobj = {
                            method  => 'getProduct_hash',
                            params  => [ $product->{'productId'} ],
                            id  => 1,
                        };

                        my $sres = $main::opsi_client->call($main::opsi_url, $callobj);
                        if (not &check_opsi_res($sres)){
                            my $tres= $sres->result;

                            my $name= xml_quote($tres->{'name'});
                            my $r= $product->{'productId'};
                            my $description= xml_quote($tres->{'description'});
                            $name=~ s/\//\\\//;
                            $description=~ s/\//\\\//;
                            $xml_msg=~ s/<xxx><\/xxx>/\n<item><productId>$r<\/productId><name>$name<\/name><description>$description<\/description><\/item>$state<xxx><\/xxx>/;
                        }
                    }
                }

            }

        } else {
            foreach my $r (@{$res->result}) {
                $callobj = {
                    method  => 'getProduct_hash',
                    params  => [ $r ],
                    id  => 1,
                };

                my $sres = $main::opsi_client->call($main::opsi_url, $callobj);
                if (not &check_opsi_res($sres)){
                    my $tres= $sres->result;

                    my $name= xml_quote($tres->{'name'});
                    my $description= xml_quote($tres->{'description'});
                    $name=~ s/\//\\\//;
                    $description=~ s/\//\\\//;
                    $xml_msg=~ s/<xxx><\/xxx>/\n<item><productId>$r<\/productId><name>$name<\/name><description>$description<\/description><\/item><xxx><\/xxx>/;
                }
            }

        }
    }
    $xml_msg=~ s/<xxx><\/xxx>//;

    # Return message
    return ( $xml_msg );
}


## @method opsi_get_product_properties
# Get product properties for a product and a specific host or gobally for a product.
# @param msg - STRING - xml message with tags productId and optional hostId
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_get_product_properties {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my ($hostId, $productId);
    my $xml_msg;

    # Build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

    # Sanity check of needed parameter
    if ((not exists $msg_hash->{'productId'}) || (@{$msg_hash->{'productId'}} != 1) || (@{$msg_hash->{'productId'}}[0] eq ref 'HASH'))  {
        &add_content2xml_hash($out_hash, "error_string", "no productId specified or productId tag invalid");
        &add_content2xml_hash($out_hash, "error", "productId");
        &main::daemon_log("$session_id ERROR: no productId specified or productId tag invalid: $msg", 1); 

        # Return message
        return ( &create_xml_string($out_hash) );
    }

    # Get productid
    $productId = @{$msg_hash->{'productId'}}[0];
    &add_content2xml_hash($out_hash, "producId", "$productId");

    # Get hostId if defined
    if (defined @{$msg_hash->{'hostId'}}[0]){
      $hostId = @{$msg_hash->{'hostId'}}[0];
      &add_content2xml_hash($out_hash, "hostId", $hostId);
    }

    # Load actions
    my $callobj = {
      method  => 'getPossibleProductActions_list',
      params  => [ $productId ],
      id  => 1,
    };
    my $res = $main::opsi_client->call($main::opsi_url, $callobj);
    if (not &check_opsi_res($res)){
      foreach my $action (@{$res->result}){
        &add_content2xml_hash($out_hash, "action", $action);
      }
    }

    # Add place holder
    &add_content2xml_hash($out_hash, "xxx", "");

    # Move to XML string
    $xml_msg= &create_xml_string($out_hash);

    # JSON Query
    if (defined $hostId){
      $callobj = {
          method  => 'getProductProperties_hash',
          params  => [ $productId, $hostId ],
          id  => 1,
      };
    } else {
      $callobj = {
          method  => 'getProductProperties_hash',
          params  => [ $productId ],
          id  => 1,
      };
    }
    $res = $main::opsi_client->call($main::opsi_url, $callobj);

    # JSON Query 2
    $callobj = {
      method  => 'getProductPropertyDefinitions_listOfHashes',
      params  => [ $productId ],
      id  => 1,
    };

    # Assemble options
    my $res2 = $main::opsi_client->call($main::opsi_url, $callobj);
    my $values = {};
    my $descriptions = {};
    if (not &check_opsi_res($res2)){
        my $r= $res2->result;

          foreach my $entr (@$r){
            # Unroll values
            my $cnv;
            if (UNIVERSAL::isa( $entr->{'values'}, "ARRAY" )){
              foreach my $v (@{$entr->{'values'}}){
                $cnv.= "<value>$v</value>";
              }
            } else {
              $cnv= $entr->{'values'};
            }
            $values->{$entr->{'name'}}= $cnv;
            $descriptions->{$entr->{'name'}}= "<description>".$entr->{'description'}."</description>";
          }
    }

    if (not &check_opsi_res($res)){
        my $r= $res->result;
        foreach my $key (keys %{$r}) {
            my $item= "\n<item>";
            my $value= $r->{$key};
            my $dsc= "";
            my $vals= "";
            if (defined $descriptions->{$key}){
              $dsc= $descriptions->{$key};
            }
            if (defined $values->{$key}){
              $vals= $values->{$key};
            }
            $item.= "<$key>$dsc<default>".xml_quote($value)."</default>$vals</$key>";
            $item.= "</item>";
            $xml_msg=~ s/<xxx><\/xxx>/$item<xxx><\/xxx>/;
        }
    }

    $xml_msg=~ s/<xxx><\/xxx>//;

    # Return message
    return ( $xml_msg );
}


## @method opsi_set_product_properties
# Set product properities for a specific host or globaly. Message needs one xml tag 'item' and within one xml tag 'name' and 'value'. The xml tags action and state are optional.
# @param msg - STRING - xml message with tags productId, action, state and optional hostId, action and state
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_set_product_properties {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my ($productId, $hostId);

    # Build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

    # Sanity check of needed parameter
    if ((not exists $msg_hash->{'productId'}) || (@{$msg_hash->{'productId'}} != 1) || (@{$msg_hash->{'productId'}}[0] eq ref 'HASH'))  {
        &add_content2xml_hash($out_hash, "error_string", "no productId specified or productId tag invalid");
        &add_content2xml_hash($out_hash, "error", "productId");
        &main::daemon_log("$session_id ERROR: no productId specified or productId tag invalid: $msg", 1); 
        return ( &create_xml_string($out_hash) );
    }
    if (not exists $msg_hash->{'item'}) {
        &add_content2xml_hash($out_hash, "error_string", "message needs one xml-tag 'item' and within the xml-tags 'name' and 'value'");
        &add_content2xml_hash($out_hash, "error", "item");
        &main::daemon_log("$session_id ERROR: message needs one xml-tag 'item' and within the xml-tags 'name' and 'value': $msg", 1); 
        return ( &create_xml_string($out_hash) );
    } else {
        if ((not exists @{$msg_hash->{'item'}}[0]->{'name'}) || (@{@{$msg_hash->{'item'}}[0]->{'name'}} != 1 )) {
            &add_content2xml_hash($out_hash, "error_string", "message needs within the xml-tag 'item' one xml-tags 'name'");
            &add_content2xml_hash($out_hash, "error", "name");
            &main::daemon_log("$session_id ERROR: message needs within the xml-tag 'item' one xml-tags 'name': $msg", 1); 
            return ( &create_xml_string($out_hash) );
        }
        if ((not exists @{$msg_hash->{'item'}}[0]->{'value'}) || (@{@{$msg_hash->{'item'}}[0]->{'value'}} != 1 )) {
            &add_content2xml_hash($out_hash, "error_string", "message needs within the xml-tag 'item' one xml-tags 'value'");
            &add_content2xml_hash($out_hash, "error", "value");
            &main::daemon_log("$session_id ERROR: message needs within the xml-tag 'item' one xml-tags 'value': $msg", 1); 
            return ( &create_xml_string($out_hash) );
        }
    }
    # if no hostId is given, set_product_properties will act on globally
    if ((exists $msg_hash->{'hostId'}) && (@{$msg_hash->{'hostId'}} > 1))  {
        &add_content2xml_hash($out_hash, "error_string", "hostId contains no or more than one values");
        &add_content2xml_hash($out_hash, "error", "hostId");
        &main::daemon_log("$session_id ERROR: hostId contains no or more than one values: $msg", 1); 
        return ( &create_xml_string($out_hash) );
    }

        
    # Get productId
    $productId =  @{$msg_hash->{'productId'}}[0];
    &add_content2xml_hash($out_hash, "productId", $productId);

    # Get hostId if defined
    if (exists $msg_hash->{'hostId'}){
        $hostId = @{$msg_hash->{'hostId'}}[0];
        &add_content2xml_hash($out_hash, "hostId", $hostId);
    }

    # Set product states if requested
    if (defined @{$msg_hash->{'action'}}[0]){
        &_set_action($productId, @{$msg_hash->{'action'}}[0], $hostId);
    }
    if (defined @{$msg_hash->{'state'}}[0]){
        &_set_state($productId, @{$msg_hash->{'state'}}[0], $hostId);
    }

    # Find properties
    foreach my $item (@{$msg_hash->{'item'}}){
        # JSON Query
        my $callobj;

        if (defined $hostId){
            $callobj = {
                method  => 'setProductProperty',
                params  => [ $productId, $item->{'name'}[0], $item->{'value'}[0], $hostId ],
                id  => 1,
            };
        } else {
            $callobj = {
                method  => 'setProductProperty',
                params  => [ $productId, $item->{'name'}[0], $item->{'value'}[0] ],
                id  => 1,
            };
        }

        my $res = $main::opsi_client->call($main::opsi_url, $callobj);
        my ($res_err, $res_err_string) = &check_opsi_res($res);

        if ($res_err){
            &main::daemon_log("$session_id ERROR: communication failed while setting '".$item->{'name'}[0]."': ".$res_err_string, 1);
            &add_content2xml_hash($out_hash, "error", $res_err_string);
        }
    }


    # Return message
    return ( &create_xml_string($out_hash) );
}


## @method opsi_get_client_hardware
# Reports client hardware inventory.
# @param msg - STRING - xml message with tag hostId
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_get_client_hardware {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my $hostId;
    my $error = 0;
    my $xml_msg;

    # Build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
      &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

    # Sanity check of needed parameter
    if ((exists $msg_hash->{'hostId'}) && (@{$msg_hash->{'hostId'}} != 1) || (@{$msg_hash->{'hostId'}}[0] eq ref 'HASH'))  {
        $error++;
        &add_content2xml_hash($out_hash, "error_string", "hostId contains no or more than one values");
        &add_content2xml_hash($out_hash, "error", "hostId");
        &main::daemon_log("$session_id ERROR: hostId contains no or more than one values: $msg", 1); 
    }

    if (not $error) {

    # Get hostId
        $hostId = @{$msg_hash->{'hostId'}}[0];
        &add_content2xml_hash($out_hash, "hostId", "$hostId");
        &add_content2xml_hash($out_hash, "xxx", "");
    }    

    # Move to XML string
    $xml_msg= &create_xml_string($out_hash);
    
    if (not $error) {

    # JSON Query
        my $callobj = {
            method  => 'getHardwareInformation_hash',
            params  => [ $hostId ],
            id  => 1,
        };

        my $res = $main::opsi_client->call($main::opsi_url, $callobj);
        if (not &check_opsi_res($res)){
            my $result= $res->result;
            if (ref $result eq "HASH") {
                foreach my $r (keys %{$result}){
                    my $item= "\n<item><id>".xml_quote($r)."</id>";
                    my $value= $result->{$r};
                    foreach my $sres (@{$value}){

                        foreach my $dres (keys %{$sres}){
                            if (defined $sres->{$dres}){
                                $item.= "<$dres>".xml_quote($sres->{$dres})."</$dres>";
                            }
                        }

                    }
                    $item.= "</item>";
                    $xml_msg=~ s%<xxx></xxx>%$item<xxx></xxx>%;

                }
            }
        }

        $xml_msg=~ s/<xxx><\/xxx>//;

    }

    # Return message
    return ( $xml_msg );
}


## @method opsi_list_clients
# Reports all Opsi clients. 
# @param msg - STRING - xml message 
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_list_clients {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];

    # Build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
      &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }
    &add_content2xml_hash($out_hash, "xxx", "");

    # Move to XML string
    my $xml_msg= &create_xml_string($out_hash);

    # JSON Query
    my $callobj = {
        method  => 'getClients_listOfHashes',
        params  => [ ],
        id  => 1,
    };
    my $res = $main::opsi_client->call($main::opsi_url, $callobj);
    if (not &check_opsi_res($res)){
        foreach my $host (@{$res->result}){
            my $item= "\n<item><name>".$host->{'hostId'}."</name>";
            if (defined($host->{'description'})){
                $item.= "<description>".xml_quote($host->{'description'})."</description>";
            }
            if (defined($host->{'notes'})){
                $item.= "<notes>".xml_quote($host->{'notes'})."</notes>";
            }
            if (defined($host->{'lastSeen'})){
                $item.= "<lastSeen>".xml_quote($host->{'lastSeen'})."</lastSeen>";
            }

            $callobj = {
              method  => 'getIpAddress',
              params  => [ $host->{'hostId'} ],
              id  => 1,
            };
            my $sres= $main::opsi_client->call($main::opsi_url, $callobj);
            if ( not &check_opsi_res($sres)){
              $item.= "<ip>".xml_quote($sres->result)."</ip>";
            }

            $callobj = {
              method  => 'getMacAddress',
              params  => [ $host->{'hostId'} ],
              id  => 1,
            };
            $sres= $main::opsi_client->call($main::opsi_url, $callobj);
            if ( not &check_opsi_res($sres)){
                $item.= "<mac>".xml_quote($sres->result)."</mac>";
            }
            $item.= "</item>";
            $xml_msg=~ s%<xxx></xxx>%$item<xxx></xxx>%;
        }
    }

    $xml_msg=~ s/<xxx><\/xxx>//;
    return ( $xml_msg );
}



## @method opsi_get_client_software
# Reports client software inventory.
# @param msg - STRING - xml message with tag hostId
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_get_client_software {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my $error = 0;
    my $hostId;
    my $xml_msg;

    # Build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
      &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

    # Sanity check of needed parameter
    if ((exists $msg_hash->{'hostId'}) && (@{$msg_hash->{'hostId'}} != 1) || (@{$msg_hash->{'hostId'}}[0] eq ref 'HASH'))  {
        $error++;
        &add_content2xml_hash($out_hash, "error_string", "hostId contains no or more than one values");
        &add_content2xml_hash($out_hash, "error", "hostId");
        &main::daemon_log("$session_id ERROR: hostId contains no or more than one values: $msg", 1); 
    }

    if (not $error) {

    # Get hostId
        $hostId = @{$msg_hash->{'hostId'}}[0];
        &add_content2xml_hash($out_hash, "hostId", "$hostId");
        &add_content2xml_hash($out_hash, "xxx", "");
    }

    $xml_msg= &create_xml_string($out_hash);

    if (not $error) {

    # JSON Query
        my $callobj = {
            method  => 'getSoftwareInformation_hash',
            params  => [ $hostId ],
            id  => 1,
        };

        my $res = $main::opsi_client->call($main::opsi_url, $callobj);
        if (not &check_opsi_res($res)){
            my $result= $res->result;
        }

        $xml_msg=~ s/<xxx><\/xxx>//;

    }

    # Return message
    return ( $xml_msg );
}


## @method opsi_get_local_products
# Reports product for given hostId or globally.
# @param msg - STRING - xml message with optional tag hostId
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_get_local_products {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my $hostId;

    # Build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }
    &add_content2xml_hash($out_hash, "xxx", "");

    # Get hostId if defined
    if ((exists $msg_hash->{'hostId'}) && (@{$msg_hash->{'hostId'}} == 1))  {
        $hostId = @{$msg_hash->{'hostId'}}[0];
        &add_content2xml_hash($out_hash, "hostId", $hostId);
    }

    # Move to XML string
    my $xml_msg= &create_xml_string($out_hash);

    # For hosts, only return the products that are or get installed
    my $callobj;
    $callobj = {
        method  => 'getLocalBootProductIds_list',
        params  => [ ],
        id  => 1,
    };

    my $res = $main::opsi_client->call($main::opsi_url, $callobj);
    my %r = ();
    for (@{$res->result}) { $r{$_} = 1 }

    if (not &check_opsi_res($res)){

        if (defined $hostId){
            $callobj = {
                method  => 'getProductStates_hash',
                params  => [ $hostId ],
                id  => 1,
            };

            my $hres = $main::opsi_client->call($main::opsi_url, $callobj);
            if (not &check_opsi_res($hres)){
                my $htmp= $hres->result->{$hostId};

                # Check state != not_installed or action == setup -> load and add
                foreach my $product (@{$htmp}){

                    if (!defined ($r{$product->{'productId'}})){
                        next;
                    }

                    # Now we've a couple of hashes...
                    if ($product->{'installationStatus'} ne "not_installed" or
                            $product->{'actionRequest'} eq "setup"){
                        my $state= "<state>".$product->{'installationStatus'}."</state><action>".$product->{'actionRequest'}."</action>";

                        $callobj = {
                            method  => 'getProduct_hash',
                            params  => [ $product->{'productId'} ],
                            id  => 1,
                        };

                        my $sres = $main::opsi_client->call($main::opsi_url, $callobj);
                        if (not &check_opsi_res($sres)){
                            my $tres= $sres->result;

                            my $name= xml_quote($tres->{'name'});
                            my $r= $product->{'productId'};
                            my $description= xml_quote($tres->{'description'});
                            $name=~ s/\//\\\//;
                            $description=~ s/\//\\\//;
                            $xml_msg=~ s/<xxx><\/xxx>/\n<item><productId>$r<\/productId><name>$name<\/name><description>$description<\/description><\/item>$state<xxx><\/xxx>/;
                        }

                    }
                }

            }

        } else {
            foreach my $r (@{$res->result}) {
                $callobj = {
                    method  => 'getProduct_hash',
                    params  => [ $r ],
                    id  => 1,
                };

                my $sres = $main::opsi_client->call($main::opsi_url, $callobj);
                if (not &check_opsi_res($sres)){
                    my $tres= $sres->result;

                    my $name= xml_quote($tres->{'name'});
                    my $description= xml_quote($tres->{'description'});
                    $name=~ s/\//\\\//;
                    $description=~ s/\//\\\//;
                    $xml_msg=~ s/<xxx><\/xxx>/\n<item><productId>$r<\/productId><name>$name<\/name><description>$description<\/description><\/item><xxx><\/xxx>/;
                }

            }

        }
    }

    $xml_msg=~ s/<xxx><\/xxx>//;

    # Retrun Message
    return ( $xml_msg );
}


## @method opsi_del_client
# Deletes a client from Opsi.
# @param msg - STRING - xml message with tag hostId
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_del_client {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my $hostId;
    my $error = 0;

    # Build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
      &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

    # Sanity check of needed parameter
    if ((exists $msg_hash->{'hostId'}) && (@{$msg_hash->{'hostId'}} != 1) || (@{$msg_hash->{'hostId'}}[0] eq ref 'HASH'))  {
        $error++;
        &add_content2xml_hash($out_hash, "error_string", "hostId contains no or more than one values");
        &add_content2xml_hash($out_hash, "error", "hostId");
        &main::daemon_log("$session_id ERROR: hostId contains no or more than one values: $msg", 1); 
    }

    if (not $error) {

    # Get hostId
        $hostId = @{$msg_hash->{'hostId'}}[0];
        &add_content2xml_hash($out_hash, "hostId", "$hostId");

    # JSON Query
        my $callobj = {
            method  => 'deleteClient',
            params  => [ $hostId ],
            id  => 1,
        };
        my $res = $main::opsi_client->call($main::opsi_url, $callobj);
    }

    # Move to XML string
    my $xml_msg= &create_xml_string($out_hash);

    # Return message
    return ( $xml_msg );
}


## @method opsi_install_client
# Set a client in Opsi to install and trigger a wake on lan message (WOL).  
# @param msg - STRING - xml message with tags hostId, macaddress
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_install_client {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];


    my ($hostId, $macaddress);

    my $error = 0;
    my @out_msg_l;

    # Build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

    # Sanity check of needed parameter
    if ((not exists $msg_hash->{'hostId'}) || (@{$msg_hash->{'hostId'}} != 1) || (@{$msg_hash->{'hostId'}}[0] eq ref 'HASH'))  {
        $error++;
        &add_content2xml_hash($out_hash, "error_string", "no hostId specified or hostId tag invalid");
        &add_content2xml_hash($out_hash, "error", "hostId");
        &main::daemon_log("$session_id ERROR: no hostId specified or hostId tag invalid: $msg", 1); 
    }
    if ((not exists $msg_hash->{'macaddress'}) || (@{$msg_hash->{'macaddress'}} != 1) || (@{$msg_hash->{'macaddress'}}[0] eq ref 'HASH') )  {
        $error++;
        &add_content2xml_hash($out_hash, "error_string", "no macaddress specified or macaddress tag invalid");
        &add_content2xml_hash($out_hash, "error", "macaddress");
        &main::daemon_log("$session_id ERROR: no macaddress specified or macaddress tag invalid: $msg", 1); 
    } else {
        if ((exists $msg_hash->{'macaddress'}) && 
                ($msg_hash->{'macaddress'}[0] =~ /^([0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2})$/i)) {  
            $macaddress = $1; 
        } else { 
            $error ++; 
            &add_content2xml_hash($out_hash, "error_string", "given mac address is not correct");
            &add_content2xml_hash($out_hash, "error", "macaddress");
            &main::daemon_log("$session_id ERROR: given mac address is not correct: $msg", 1); 
        }
    }

    if (not $error) {

    # Get hostId
        $hostId = @{$msg_hash->{'hostId'}}[0];
        &add_content2xml_hash($out_hash, "hostId", "$hostId");

        # Load all products for this host with status != "not_installed" or actionRequest != "none"
        my $callobj = {
            method  => 'getProductStates_hash',
            params  => [ $hostId ],
            id  => 1,
        };

        my $hres = $main::opsi_client->call($main::opsi_url, $callobj);
        if (not &check_opsi_res($hres)){
            my $htmp= $hres->result->{$hostId};

            # check state != not_installed or action == setup -> load and add
            foreach my $product (@{$htmp}){
                # Now we've a couple of hashes...
                if ($product->{'installationStatus'} ne "not_installed" or
                        $product->{'actionRequest'} ne "none"){

                    # Do an action request for all these -> "setup".
                    $callobj = {
                        method  => 'setProductActionRequest',
                        params  => [ $product->{'productId'}, $hostId, "setup" ],
                        id  => 1,
                    };
                    my $res = $main::opsi_client->call($main::opsi_url, $callobj);
                    my ($res_err, $res_err_string) = &check_opsi_res($res);
                    if ($res_err){
                        &main::daemon_log("$session_id ERROR: cannot set product action request for '$hostId': ".$product->{'productId'}, 1);
                    } else {
                        &main::daemon_log("$session_id INFO: requesting 'setup' for '".$product->{'productId'}."' on $hostId", 1);
                    }
                }
            }
        }
        push(@out_msg_l, &create_xml_string($out_hash));
    

    # Build wakeup message for client
        if (not $error) {
            my $wakeup_hash = &create_xml_hash("trigger_wake", "GOSA", "KNOWN_SERVER");
            &add_content2xml_hash($wakeup_hash, 'macaddress', $macaddress);
            my $wakeup_msg = &create_xml_string($wakeup_hash);
            push(@out_msg_l, $wakeup_msg);

            # invoke trigger wake for this gosa-si-server
            &main::server_server_com::trigger_wake($wakeup_msg, $wakeup_hash, $session_id);
        }
    }
    
    # Return messages
    return @out_msg_l;
}


## @method _set_action
# Set action for an Opsi client
# @param product - STRING - Opsi product
# @param action - STRING - action
# @param hostId - STRING - Opsi hostId
sub _set_action {
  my $product= shift;
  my $action = shift;
  my $hostId = shift;
  my $callobj;

  $callobj = {
    method  => 'setProductActionRequest',
    params  => [ $product, $hostId, $action],
    id  => 1,
  };

  $main::opsi_client->call($main::opsi_url, $callobj);
}

## @method _set_state
# Set state for an Opsi client
# @param product - STRING - Opsi product
# @param action - STRING - state
# @param hostId - STRING - Opsi hostId
sub _set_state {
  my $product = shift;
  my $state = shift;
  my $hostId = shift;
  my $callobj;

  $callobj = {
    method  => 'setProductState',
    params  => [ $product, $hostId, $state ],
    id  => 1,
  };

  $main::opsi_client->call($main::opsi_url, $callobj);
}

################################
#
# @brief Create a license pool at Opsi server.
# @param licensePoolId The name of the pool (optional). 
# @param description The description of the pool (optional).
# @param productIds A list of assigned porducts of the pool (optional). 
# @param windowsSoftwareIds A list of windows software IDs associated to the pool (optional). 
#
sub opsi_createLicensePool {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
	my $out_hash;
	my $licensePoolId = defined $msg_hash->{'licensePoolId'} ? @{$msg_hash->{'licensePoolId'}}[0] : undef;
	my $description = defined $msg_hash->{'description'} ? @{$msg_hash->{'description'}}[0] : undef;
	my @productIds = defined $msg_hash->{'productIds'} ? $msg_hash->{'productIds'} : undef;
	my @windowsSoftwareIds = defined $msg_hash->{'windowsSoftwareIds'} ? $msg_hash->{'windowsSoftwareIds'} : undef;

	# Create license Pool
    my $callobj = {
        method  => 'createLicensePool',
        params  => [ $licensePoolId, $description, @productIds, @windowsSoftwareIds],
        id  => 1,
    };
    my $res = $main::opsi_client->call($main::opsi_url, $callobj);

	# Check Opsi error
	my ($res_error, $res_error_str) = &check_opsi_res($res);
	if ($res_error){
		# Create error message
		&main::daemon_log("$session_id ERROR: cannot create license pool at Opsi server: ".$res_error_str, 1);
		$out_hash = &main::create_xml_hash("error_$header", $main::server_address, $source, $res_error_str);
		return ( &create_xml_string($out_hash) );
	}

	# Create function result message
	$out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source, $res->result);

	return ( &create_xml_string($out_hash) );
}

################################
#
# @brief Return licensePoolId, description, productIds and windowsSoftwareIds for all found license pools.
#
sub opsi_getLicensePools_listOfHashes {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
	my $out_hash;

	# Fetch infos from Opsi server
    my $callobj = {
        method  => 'getLicensePools_listOfHashes',
        params  => [ ],
        id  => 1,
    };
    my $res = $main::opsi_client->call($main::opsi_url, $callobj);

	# Check Opsi error
	my ($res_error, $res_error_str) = &check_opsi_res($res);
	if ($res_error){
		# Create error message
		&main::daemon_log("$session_id ERROR: cannot get license pool ID list from Opsi server: ".$res_error_str, 1);
		$out_hash = &main::create_xml_hash("error_$header", $main::server_address, $source, $res_error_str);
		return ( &create_xml_string($out_hash) );
	}

	# Create function result message
	my $res_hash = { 'hit'=> [] };
	foreach my $licensePool ( @{$res->result}) {
		my $licensePool_hash = { 'licensePoolId' => [$licensePool->{'licensePoolId'}],
			'description' => [$licensePool->{'description'}],
			'productIds' => $licensePool->{'productIds'},
			'windowsSoftwareIds' => $licensePool->{'windowsSoftwareIds'},
			};
		push( @{$res_hash->{hit}}, $licensePool_hash );
	}

	# Create function result message
	$out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
	$out_hash->{result} = [$res_hash];

	return ( &create_xml_string($out_hash) );
}

################################
#
# @brief Return productIds, windowsSoftwareIds and description for a given licensePoolId
# @param licensePoolId The name of the pool. 
#
sub opsi_getLicensePool_hash {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $licensePoolId;
	my $out_hash;

	# Check input sanity
	if (&_check_xml_tag_is_ok ($msg_hash, 'licensePoolId')) {
		$licensePoolId = @{$msg_hash->{'licensePoolId'}}[0];
	} else {
		return ( &_give_feedback($msg, $msg_hash, $session_id, $_) );
	}

	# Fetch infos from Opsi server
    my $callobj = {
        method  => 'getLicensePool_hash',
        params  => [ $licensePoolId ],
        id  => 1,
    };
    my $res = $main::opsi_client->call($main::opsi_url, $callobj);

	# Check Opsi error
	my ($res_error, $res_error_str) = &check_opsi_res($res);
	if ($res_error){
		# Create error message
		&main::daemon_log("$session_id ERROR: cannot get license pool from Opsi server: ".$res_error_str, 1);
		$out_hash = &main::create_xml_hash("error_$header", $main::server_address, $source);
		&add_content2xml_hash($out_hash, "error", $res_error_str);
		return ( &create_xml_string($out_hash) );
	}

	# Create function result message
	$out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
	&add_content2xml_hash($out_hash, "licensePoolId", $res->result->{'licensePoolId'});
	&add_content2xml_hash($out_hash, "description", $res->result->{'description'});
	map(&add_content2xml_hash($out_hash, "productIds", "$_"), @{ $res->result->{'productIds'} });
	map(&add_content2xml_hash($out_hash, "windowsSoftwareIds", "$_"), @{ $res->result->{'windowsSoftwareIds'} });

	return ( &create_xml_string($out_hash) );
}

################################
#
# @brief Returns softwareLicenseId, notes, licenseKey, hostId and licensePoolId for optional given licensePoolId and hostId
# @param hostid Something like client_1.intranet.mydomain.de (optional).
# @param licensePoolId The name of the pool (optional). 
# 
sub opsi_getSoftwareLicenseUsages_listOfHashes {
	my ($msg, $msg_hash, $session_id) = @_;
	my $header = @{$msg_hash->{'header'}}[0];
	my $source = @{$msg_hash->{'source'}}[0];
	my $target = @{$msg_hash->{'target'}}[0];
	my $licensePoolId = defined $msg_hash->{'licensePoolId'} ? @{$msg_hash->{'licensePoolId'}}[0] : undef;
	my $hostId = defined $msg_hash->{'hostId'} ? @{$msg_hash->{'hostId'}}[0] : undef;
	my $out_hash;

	# Fetch information from Opsi server
	my $callobj = {
		method  => 'getSoftwareLicenseUsages_listOfHashes',
		params  => [  $hostId, $licensePoolId ],
		id  => 1,
	};
	my $res = $main::opsi_client->call($main::opsi_url, $callobj);

	# Check Opsi error
	my ($res_error, $res_error_str) = &check_opsi_res($res);
	if ($res_error){
		# Create error message
		&main::daemon_log("$session_id ERROR: cannot fetch software licenses from license pool '$licensePoolId': ".$res_error_str, 1);
		$out_hash = &main::create_xml_hash("error_$header", $main::server_address, $source, $res_error_str);
		return ( &create_xml_string($out_hash) );
	}

	# Parse Opsi result
	my $res_hash = { 'hit'=> [] };
	foreach my $license ( @{$res->result}) {
		my $license_hash = { 'softwareLicenseId' => [$license->{'softwareLicenseId'}],
			'notes' => [$license->{'notes'}],
			'licenseKey' => [$license->{'licenseKey'}],
			'hostId' => [$license->{'hostId'}],
			'licensePoolId' => [$license->{'licensePoolId'}],
			};
		push( @{$res_hash->{hit}}, $license_hash );
	}

	# Create function result message
	$out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
	$out_hash->{result} = [$res_hash];

	return ( &create_xml_string($out_hash) );
}

################################
#
# @brief Returns expirationDate, boundToHost, maxInstallation, licenseTyp, licensePoolIds and licenseKeys for a given softwareLicense ID.
# @param softwareLicenseId Identificator of a license.
#
sub opsi_getSoftwareLicense_hash {
	my ($msg, $msg_hash, $session_id) = @_;
	my $header = @{$msg_hash->{'header'}}[0];
	my $source = @{$msg_hash->{'source'}}[0];
	my $target = @{$msg_hash->{'target'}}[0];
	my $softwareLicenseId;
	my $out_hash;

	# Check input sanity
	if (&_check_xml_tag_is_ok ($msg_hash, 'softwareLicenseId')) {
		$softwareLicenseId = @{$msg_hash->{'softwareLicenseId'}}[0];
	} else {
		return ( &_give_feedback($msg, $msg_hash, $session_id, $_) );
	}

	my $callobj = {
		method  => 'getSoftwareLicense_hash',
		params  => [ $softwareLicenseId ],
		id  => 1,
	};
	my $res = $main::opsi_client->call($main::opsi_url, $callobj);

	# Check Opsi error
	my ($res_error, $res_error_str) = &check_opsi_res($res);
	if ($res_error){
		# Create error message
		&main::daemon_log("$session_id ERROR: cannot fetch information for license '$softwareLicenseId': ".$res_error_str, 1);
		$out_hash = &main::create_xml_hash("error_$header", $main::server_address, $source, $res_error_str);
		return ( &create_xml_string($out_hash) );
	}
	
	# Create function result message
	$out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
	&add_content2xml_hash($out_hash, "expirationDate", $res->result->{'expirationDate'});
	&add_content2xml_hash($out_hash, "boundToHost", $res->result->{'boundToHost'});
	&add_content2xml_hash($out_hash, "maxInstallations", $res->result->{'maxInstallations'});
	&add_content2xml_hash($out_hash, "licenseTyp", $res->result->{'licenseTyp'});
	foreach my $licensePoolId ( @{$res->result->{'licensePoolIds'}}) {
		&add_content2xml_hash($out_hash, "licensePoolId", $licensePoolId);
		&add_content2xml_hash($out_hash, $licensePoolId, $res->result->{'licenseKeys'}->{$licensePoolId});
	}

	return ( &create_xml_string($out_hash) );
}

################################
#
# @brief Delete licnese pool by license pool ID. A pool can only be deleted if there are no software licenses bound to the pool. 
# The fixed parameter deleteLicenses=True specifies that all software licenses bound to the pool are being deleted. 
# @param licensePoolId The name of the pool. 
#
sub opsi_deleteLicensePool {
	my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $licensePoolId;
	my $out_hash;

	# Check input sanity
	if (&_check_xml_tag_is_ok ($msg_hash, 'licensePoolId')) {
		$licensePoolId = @{$msg_hash->{'licensePoolId'}}[0];
	} else {
		return ( &_give_feedback($msg, $msg_hash, $session_id, $_) );
	}

	# Fetch softwareLicenseIds used in license pool
	# This has to be done because function deleteLicensePool deletes the pool and the corresponding software licenses
	# but not the license contracts of the software licenses. In our case each software license has exactly one license contract. 
	my $callobj = {
		method  => 'getSoftwareLicenses_listOfHashes',
		params  => [ ],
		id  => 1,
	};
	my $res = $main::opsi_client->call($main::opsi_url, $callobj);

	# Keep list of licenseContractIds in mind to delete it after the deletion of the software licenses
	my @lCI_toBeDeleted;
	foreach my $softwareLicenseHash ( @{$res->result} ) {
		if ((@{$softwareLicenseHash->{'licensePoolIds'}} == 0) || (@{$softwareLicenseHash->{'licensePoolIds'}}[0] ne $licensePoolId)) { 
			next; 
		}  
		push (@lCI_toBeDeleted, $softwareLicenseHash->{'licenseContractId'});
	}

	# Delete license pool at Opsi server
    $callobj = {
        method  => 'deleteLicensePool',
        params  => [ $licensePoolId, 'deleteLicenses=True'  ],
        id  => 1,
    };
    $res = $main::opsi_client->call($main::opsi_url, $callobj);
	my ($res_error, $res_error_str) = &check_opsi_res($res);
	if ($res_error){
		# Create error message
		&main::daemon_log("$session_id ERROR: cannot delete license pool at Opsi server: ".$res_error_str, 1);
		$out_hash = &main::create_xml_hash("error_$header", $main::server_address, $source, $res_error_str);
		return ( &create_xml_string($out_hash) );
	} 

	# Delete each license contract connected with the license pool
	foreach my $licenseContractId ( @lCI_toBeDeleted ) {
		my $callobj = {
			method  => 'deleteLicenseContract',
			params  => [ $licenseContractId ],
			id  => 1,
		};
		my $res = $main::opsi_client->call($main::opsi_url, $callobj);
		my ($res_error, $res_error_str) = &check_opsi_res($res);
		if ($res_error){
			# Create error message
			&main::daemon_log("$session_id ERROR: cannot delete license contract '$licenseContractId' connected with license pool '$licensePoolId' at Opsi server: ".$res_error_str, 1);
			$out_hash = &main::create_xml_hash("error_$header", $main::server_address, $source, $res_error_str);
			return ( &create_xml_string($out_hash) );
		}
	}

	# Create function result message
	$out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
	
	return ( &create_xml_string($out_hash) );
}

################################
#
# @brief Create a license contract, create a software license and add the software license to the license pool
# @param licensePoolId The name of the pool the license should be assigned.
# @param licenseKey The license key.
# @param partner Name of the license partner (optional).
# @param conclusionDate Date of conclusion of license contract (optional)
# @param notificationDate Date of notification that license is running out soon (optional).
# @param notes This is the place for some notes (optional)
# @param softwareLicenseId Identificator of a license (optional).
# @param licenseTyp Typ of a licnese, either "OEM", "VOLUME" or "RETAIL" (optional).
# @param maxInstallations The number of clients use this license (optional). 
# @param boundToHost The name of the client the license is bound to (optional).
# @param expirationDate The date when the license is running down (optional). 
#
sub opsi_createLicense {
	my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
	my $partner = defined $msg_hash->{'partner'} ? @{$msg_hash->{'partner'}}[0] : undef;
	my $conclusionDate = defined $msg_hash->{'conclusionDate'} ? @{$msg_hash->{'conclusionDate'}}[0] : undef;
	my $notificationDate = defined $msg_hash->{'notificationDate'} ? @{$msg_hash->{'notificationDate'}}[0] : undef;
	my $notes = defined $msg_hash->{'notes'} ? @{$msg_hash->{'notes'}}[0] : undef;
	my $licenseContractId;
	my $softwareLicenseId = defined $msg_hash->{'licenseId'} ? @{$msg_hash->{'licenseId'}}[0] : undef;
	my $licenseType = defined $msg_hash->{'licenseType'} ? @{$msg_hash->{'licenseType'}}[0] : undef;
	my $maxInstallations = defined $msg_hash->{'maxInstallations'} ? @{$msg_hash->{'maxInstallations'}}[0] : undef;
	my $boundToHost = defined $msg_hash->{'boundToHost'} ? @{$msg_hash->{'boundToHost'}}[0] : undef;
	my $expirationDate = defined $msg_hash->{'expirationDate'} ? @{$msg_hash->{'expirationDate'}}[0] : undef;
	my $licensePoolId;
	my $licenseKey;
	my $out_hash;

	# Check input sanity
	if (&_check_xml_tag_is_ok ($msg_hash, 'licenseKey')) {
		$licenseKey = @{$msg_hash->{'licenseKey'}}[0];
	} else {
		return ( &_give_feedback($msg, $msg_hash, $session_id, $_) );
	}
	if (&_check_xml_tag_is_ok ($msg_hash, 'licensePoolId')) {
		$licensePoolId = @{$msg_hash->{'licensePoolId'}}[0];
	} else {
		return ( &_give_feedback($msg, $msg_hash, $session_id, $_) );
	}
	if ((defined $licenseType) && (not exists $licenseTyp_hash->{$licenseType})) {
		return ( &_give_feedback($msg, $msg_hash, $session_id, "The typ of a license can be either 'OEM', 'VOLUME' or 'RETAIL'."));
	}

	# Create license contract at Opsi server
    my $callobj = {
        method  => 'createLicenseContract',
        params  => [ undef, $partner, $conclusionDate, $notificationDate, undef, $notes ],
        id  => 1,
    };
    my $res = $main::opsi_client->call($main::opsi_url, $callobj);

	# Check Opsi error
	my ($res_error, $res_error_str) = &check_opsi_res($res);
	if ($res_error){
		# Create error message
		&main::daemon_log("$session_id ERROR: cannot create license contract at Opsi server: ".$res_error_str, 1);
		$out_hash = &main::create_xml_hash("error_$header", $main::server_address, $source, $res_error_str);
		return ( &create_xml_string($out_hash) );
	}
	
	$licenseContractId = $res->result;

	# Create software license at Opsi server
    $callobj = {
        method  => 'createSoftwareLicense',
        params  => [ $softwareLicenseId, $licenseContractId, $licenseType, $maxInstallations, $boundToHost, $expirationDate ],
        id  => 1,
    };
    $res = $main::opsi_client->call($main::opsi_url, $callobj);

	# Check Opsi error
	($res_error, $res_error_str) = &check_opsi_res($res);
	if ($res_error){
		# Create error message
		&main::daemon_log("$session_id ERROR: cannot create software license at Opsi server: ".$res_error_str, 1);
		$out_hash = &main::create_xml_hash("error_$header", $main::server_address, $source, $res_error_str);
		return ( &create_xml_string($out_hash) );
	}

	$softwareLicenseId = $res->result;

	# Add software license to license pool
	$callobj = {
        method  => 'addSoftwareLicenseToLicensePool',
        params  => [ $softwareLicenseId, $licensePoolId, $licenseKey ],
        id  => 1,
    };
    $res = $main::opsi_client->call($main::opsi_url, $callobj);

	# Check Opsi error
	($res_error, $res_error_str) = &check_opsi_res($res);
	if ($res_error){
		# Create error message
		&main::daemon_log("$session_id ERROR: cannot add software license to license pool at Opsi server: ".$res_error_str, 1);
		$out_hash = &main::create_xml_hash("error_$header", $main::server_address, $source, $res_error_str);
		return ( &create_xml_string($out_hash) );
	}

	# Create function result message
	$out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
	
	return ( &create_xml_string($out_hash) );
}

################################
#
# @brief Assign a software license to a host
# @param hostid Something like client_1.intranet.mydomain.de
# @param licensePoolId The name of the pool.
#
sub opsi_assignSoftwareLicenseToHost {
	my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
	my $hostId;
	my $licensePoolId;

	# Check input sanity
	if (&_check_xml_tag_is_ok ($msg_hash, 'hostId')) {
		$hostId = @{$msg_hash->{'hostId'}}[0];
	} else {
		return ( &_give_feedback($msg, $msg_hash, $session_id, $_) );
	}
	if (&_check_xml_tag_is_ok ($msg_hash, 'licensePoolId')) {
		$licensePoolId = @{$msg_hash->{'licensePoolId'}}[0];
	} else {
		return ( &_give_feedback($msg, $msg_hash, $session_id, $_) );
	}

	# Assign a software license to a host
	my $callobj = {
        method  => 'getAndAssignSoftwareLicenseKey',
        params  => [ $hostId, $licensePoolId ],
        id  => 1,
    };
    my $res = $main::opsi_client->call($main::opsi_url, $callobj);

	# Check Opsi error
	my ($res_error, $res_error_str) = &check_opsi_res($res);
	if ($res_error){
		# Create error message
		&main::daemon_log("$session_id ERROR: cannot assign a software license to a host at Opsi server: ".$res_error_str, 1);
		my $out_hash = &main::create_xml_hash("error_$header", $main::server_address, $source, $res_error_str);
		return ( &create_xml_string($out_hash) );
	}

	# Create function result message
	my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
	
	return ( &create_xml_string($out_hash) );
}

################################
#
# @brief Unassign a software license from a host.
# @param hostid Something like client_1.intranet.mydomain.de
# @param licensePoolId The name of the pool.
#
sub opsi_unassignSoftwareLicenseFromHost {
	my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
	my $hostId;
	my $licensePoolId;

	# Check input sanity
	if (&_check_xml_tag_is_ok ($msg_hash, 'hostId')) {
		$hostId = @{$msg_hash->{'hostId'}}[0];
	} else {
		return ( &_give_feedback($msg, $msg_hash, $session_id, $_) );
	}
	if (&_check_xml_tag_is_ok ($msg_hash, 'licensePoolId')) {
		$licensePoolId = @{$msg_hash->{'licensePoolId'}}[0];
	} else {
		return ( &_give_feedback($msg, $msg_hash, $session_id, $_) );
	}

	# Unassign a software license from a host
	my $callobj = {
        method  => 'deleteSoftwareLicenseUsage',
        params  => [ $hostId, '', $licensePoolId ],
        id  => 1,
    };
    my $res = $main::opsi_client->call($main::opsi_url, $callobj);

	# Check Opsi error
	my ($res_error, $res_error_str) = &check_opsi_res($res);
	if ($res_error){
		# Create error message
		&main::daemon_log("$session_id ERROR: cannot unassign a software license from a host at Opsi server: ".$res_error_str, 1);
		my $out_hash = &main::create_xml_hash("error_$header", $main::server_address, $source, $res_error_str);
		return ( &create_xml_string($out_hash) );
	}

	# Create function result message
	my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
	
	return ( &create_xml_string($out_hash) );
}

################################
#
# @brief Unassign all software licenses from a host
# @param hostid Something like client_1.intranet.mydomain.de
#
sub opsi_unassignAllSoftwareLicensesFromHost {
	my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
	my $hostId;

	# Check input sanity
	if (&_check_xml_tag_is_ok ($msg_hash, 'hostId')) {
		$hostId = @{$msg_hash->{'hostId'}}[0];
	} else {
		return ( &_give_feedback($msg, $msg_hash, $session_id, $_) );
	}

	# Unassign all software licenses from a host
	my $callobj = {
        method  => 'deleteAllSoftwareLicenseUsages',
        params  => [ $hostId ],
        id  => 1,
    };
    my $res = $main::opsi_client->call($main::opsi_url, $callobj);

	# Check Opsi error
	my ($res_error, $res_error_str) = &check_opsi_res($res);
	if ($res_error){
		# Create error message
		&main::daemon_log("$session_id ERROR: cannot unassign a software license from a host at Opsi server: ".$res_error_str, 1);
		my $out_hash = &main::create_xml_hash("error_$header", $main::server_address, $source, $res_error_str);
		return ( &create_xml_string($out_hash) );
	}

	# Create function result message
	my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
	
	return ( &create_xml_string($out_hash) );
}


################################
#
# @brief Returns the assigned licensePoolId and licenses, how often the product is installed and at which host
# and the number of max and remaining installations for a given OPSI product.
# @param productId Identificator of an OPSI product.
#	
sub opsi_getLicenseInformationForProduct {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
	my $productId;
	my $out_hash;

	# Check input sanity
	if (&_check_xml_tag_is_ok ($msg_hash, 'productId')) {
		$productId = @{$msg_hash->{'productId'}}[0];
	} else {
		return ( &_give_feedback($msg, $msg_hash, $session_id, $_) );
	}

	# Fetch infos from Opsi server
    my $callobj = {
        method  => 'getLicensePoolId',
        params  => [ $productId ],
        id  => 1,
    };
    my $res = $main::opsi_client->call($main::opsi_url, $callobj);

	# Check Opsi error
	my ($res_error, $res_error_str) = &check_opsi_res($res);
	if ($res_error){
		# Create error message
		&main::daemon_log("$session_id ERROR: cannot get license pool for product '$productId' : ".$res_error_str, 1);
		$out_hash = &main::create_xml_hash("error_$header", $main::server_address, $source, $res_error_str);
		return ( &create_xml_string($out_hash) );
	} 
	
	my $licensePoolId = $res->result;

	# Fetch statistic information for given pool ID
	$callobj = {
		method  => 'getLicenseStatistics_hash',
		params  => [ ],
		id  => 1,
	};
	$res = $main::opsi_client->call($main::opsi_url, $callobj);

	# Check Opsi error
	($res_error, $res_error_str) = &check_opsi_res($res);
	if ($res_error){
		# Create error message
		&main::daemon_log("$session_id ERROR: cannot get statistic informations for license pools : ".$res_error_str, 1);
		$out_hash = &main::create_xml_hash("error_$header", $main::server_address, $source, $res_error_str);
		return ( &create_xml_string($out_hash) );
	}

	# Create function result message
	$out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
	&add_content2xml_hash($out_hash, "licensePoolId", $licensePoolId);
	&add_content2xml_hash($out_hash, "licenses", $res->result->{$licensePoolId}->{'licenses'});
	&add_content2xml_hash($out_hash, "usageCount", $res->result->{$licensePoolId}->{'usageCount'});
	&add_content2xml_hash($out_hash, "maxInstallations", $res->result->{$licensePoolId}->{'maxInstallations'});
	&add_content2xml_hash($out_hash, "remainingInstallations", $res->result->{$licensePoolId}->{'remainingInstallations'});
	map(&add_content2xml_hash($out_hash, "usedBy", "$_"), @{ $res->result->{$licensePoolId}->{'usedBy'}});

	return ( &create_xml_string($out_hash) );
}


################################
#
# @brief
# @param 
#	
sub opsi_getPool {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];

	# Check input sanity
	my $licensePoolId;
	if (&_check_xml_tag_is_ok ($msg_hash, 'licensePoolId')) {
		$licensePoolId = @{$msg_hash->{'licensePoolId'}}[0];
	} else {
		return ( &_give_feedback($msg, $msg_hash, $session_id, $_) );
	}

	# Create hash for the answer
	my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);

	# Call Opsi
	my ($res, $err) = &_getLicensePool_hash( 'licensePoolId'=> $licensePoolId );
	if ($err){
		return &_giveErrorFeedback($msg_hash, "cannot get license pool from Opsi server: ".$res, $session_id);
	}
	# Add data to outgoing hash
	&add_content2xml_hash($out_hash, "licensePoolId", $res->{'licensePoolId'});
	&add_content2xml_hash($out_hash, "description", $res->{'description'});
	map(&add_content2xml_hash($out_hash, "productIds", "$_"), @{ $res->{'productIds'} });
	map(&add_content2xml_hash($out_hash, "windowsSoftwareIds", "$_"), @{ $res->{'windowsSoftwareIds'} });


	# Call Opsi two times
	my ($usages_res, $usages_err) = &_getSoftwareLicenseUsages_listOfHashes('licensePoolId'=>$licensePoolId);
	if ($usages_err){
		return &_giveErrorFeedback($msg_hash, "cannot get software license usage information from Opsi server: ".$usages_res, $session_id);
	}
	my ($licenses_res, $licenses_err) = &_getSoftwareLicenses_listOfHashes();
	if ($licenses_err){
		return &_giveErrorFeedback($msg_hash, "cannot get software license information from Opsi server: ".$licenses_res, $session_id);
	}

	# Add data to outgoing hash
	# Parse through all software licenses and select those associated to the pool
	my $res_hash = { 'hit'=> [] };
	foreach my $license ( @$licenses_res) {
		# Each license hash has a list of licensePoolIds so go through this list and search for matching licensePoolIds
		my $found = 0;
		my @licensePoolIds_list = @{$license->{licensePoolIds}};
		foreach my $lPI ( @licensePoolIds_list) {
			if ($lPI eq $licensePoolId) { $found++ }
		}
		if (not $found ) { next; };
		# Found matching licensePoolId
		my $license_hash = { 'softwareLicenseId' => [$license->{'softwareLicenseId'}],
			'licenseKeys' => {},
			'expirationDate' => [$license->{'expirationDate'}],
			'boundToHost' => [$license->{'boundToHost'}],
			'maxInstallations' => [$license->{'maxInstallations'}],
			'licenseType' => [$license->{'licenseType'}],
			'licenseContractId' => [$license->{'licenseContractId'}],
			'licensePoolIds' => [],
			'hostIds' => [],
			};
		foreach my $licensePoolId (@{ $license->{'licensePoolIds'}}) {
			push( @{$license_hash->{'licensePooIds'}}, $licensePoolId);
			$license_hash->{licenseKeys}->{$licensePoolId} =  [ $license->{'licenseKeys'}->{$licensePoolId} ];
		}
		foreach my $usage (@$usages_res) {
			# Search for hostIds with matching softwareLicenseId
			if ($license->{'softwareLicenseId'} eq $usage->{'softwareLicenseId'}) {
				push( @{ $license_hash->{hostIds}}, $usage->{hostId});
			}
		}

		push( @{$res_hash->{hit}}, $license_hash );
	}
	$out_hash->{licenses} = [$res_hash];

    return ( &create_xml_string($out_hash) );
}


################################
#
# @brief Removes at first the software license from license pool and than deletes the software license. 
# Attention, the software license has to exists otherwise it will lead to an Opsi internal server error.
# @param softwareLicenseId 
# @param licensePoolId
#
sub opsi_removeLicense {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];

	# Check input sanity
	my $softwareLicenseId;
	if (&_check_xml_tag_is_ok ($msg_hash, 'softwareLicenseId')) {
		$softwareLicenseId = @{$msg_hash->{'softwareLicenseId'}}[0];
	} else {
		return ( &_give_feedback($msg, $msg_hash, $session_id, $_) );
	}
	my $licensePoolId;
	if (&_check_xml_tag_is_ok ($msg_hash, 'licensePoolId')) {
		$licensePoolId = @{$msg_hash->{'licensePoolId'}}[0];
	} else {
		return ( &_give_feedback($msg, $msg_hash, $session_id, $_) );
	}
	
	# Call Opsi
	my ($res, $err) = &_removeSoftwareLicenseFromLicensePool( 'licensePoolId' => $licensePoolId, 'softwareLicenseId' => $softwareLicenseId );
	if ($err){
		return &_giveErrorFeedback($msg_hash, "cannot delete software license from pool: ".$res, $session_id);
	}

	# Call Opsi
	($res, $err) = &_deleteSoftwareLicense( 'softwareLicenseId'=>$softwareLicenseId );
	if ($err){
		return &_giveErrorFeedback($msg_hash, "cannot delete software license from Opsi server: ".$res, $session_id);
	}

	# Create hash for the answer
	my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
	return ( &create_xml_string($out_hash) );
}

sub opsi_test {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
	my $pram1 = @{$msg_hash->{'productId'}}[0];

print STDERR Dumper $pram1;

	# Fetch infos from Opsi server
    my $callobj = {
        method  => 'getLicensePoolId',
        params  => [ $pram1 ],
        id  => 1,
    };
    my $res = $main::opsi_client->call($main::opsi_url, $callobj);

	print STDERR Dumper $res;
	return ();
}

sub _giveErrorFeedback {
	my ($msg_hash, $err_string, $session_id) = @_;
	&main::daemon_log("$session_id ERROR: $err_string", 1);
	my $out_hash = &main::create_xml_hash("error", $main::server_address, @{$msg_hash->{source}}[0], $err_string);
	return ( &create_xml_string($out_hash) );
}


sub _getLicensePool_hash {
	my %arg = (
		'licensePoolId' => undef,
		@_,
	);

	if (not defined $arg{licensePoolId} ) { 
		return ("function requires licensePoolId as parameter", 1);
	}

	# Fetch pool infos from Opsi server
    my $callobj = {
        method  => 'getLicensePool_hash',
        params  => [ $arg{licensePoolId} ],
        id  => 1,
    };
    my $res = $main::opsi_client->call($main::opsi_url, $callobj);

	# Check Opsi error
	my ($res_error, $res_error_str) = &check_opsi_res($res);
	if ($res_error){ return ( (caller(0))[3]." : ".$res_error_str, 1 ); }

	return ($res->result, 0);
}

sub _getSoftwareLicenses_listOfHashes {
	# Fetch licenses associated to the given pool
	my $callobj = {
		method  => 'getSoftwareLicenses_listOfHashes',
		params  => [ ],
		id  => 1,
	};
	my $res = $main::opsi_client->call($main::opsi_url, $callobj);

	# Check Opsi error
	my ($res_error, $res_error_str) = &check_opsi_res($res);
	if ($res_error){ return ( (caller(0))[3]." : ".$res_error_str, 1 ); }

	return ($res->result, 0);
}

sub _getSoftwareLicenseUsages_listOfHashes {
	my %arg = (
			'hostId' => "",
			'licensePoolId' => "",
			@_,
			);

	# Fetch pool infos from Opsi server
	my $callobj = {
		method  => 'getSoftwareLicenseUsages_listOfHashes',
		params  => [ $arg{hostId}, $arg{licensePoolId} ],
		id  => 1,
	};
	my $res = $main::opsi_client->call($main::opsi_url, $callobj);

	# Check Opsi error
	my ($res_error, $res_error_str) = &check_opsi_res($res);
	if ($res_error){ return ( (caller(0))[3]." : ".$res_error_str, 1 ); }

	return ($res->result, 0);
}

sub _removeSoftwareLicenseFromLicensePool {
	my %arg = (
		'softwareLicenseId' => undef,
		'licensePoolId' => undef,
		@_,
		);

	if (not defined $arg{softwareLicenseId} ) { 
		return ("function requires softwareLicenseId as parameter", 1);
		}
		if (not defined $arg{licensePoolId} ) { 
		return ("function requires licensePoolId as parameter", 1);
	}

	# Remove software license from license pool
	my $callobj = {
		method  => 'removeSoftwareLicenseFromLicensePool',
		params  => [ $arg{softwareLicenseId}, $arg{licensePoolId} ],
		id  => 1,
	};
	my $res = $main::opsi_client->call($main::opsi_url, $callobj);

	# Check Opsi error
	my ($res_error, $res_error_str) = &check_opsi_res($res);
	if ($res_error){ return ( (caller(0))[3]." : ".$res_error_str, 1 ); }

	return ($res->result, 0);
}

sub _deleteSoftwareLicense {
	my %arg = (
		'softwareLicenseId' => undef,
		'removeFromPools' => "",
		@_,
		);

	if (not defined $arg{softwareLicenseId} ) { 
		return ("function requires softwareLicenseId as parameter", 1);
	}

	# Fetch
	my $callobj = {
		method  => 'deleteSoftwareLicense',
		params  => [ $arg{softwareLicenseId}, $arg{removeFromPools} ],
		id  => 1,
	};
	my $res = $main::opsi_client->call($main::opsi_url, $callobj);

	# Check Opsi error
	my ($res_error, $res_error_str) = &check_opsi_res($res);
	if ($res_error){ return ( (caller(0))[3]." : ".$res_error_str, 1 ); }

	return ($res->result, 0);
}

1;
