<?php

class groupware extends plugin
{
    var $plHeadline     = "Mail";
    var $plDescription  = "GOsa mail extension.";
    var $view_logged = FALSE;

    var $attributes = array(
            "mailAddress",
            "mailLocation",
            "quotaUsage",
            "quotaSize",
            "alternateAddresses",
            "forwardingAddresses",
            "vacationEnabled",
            "vacationStart",
            "vacationStop",
            "vacationMessage",
            "mailBoxWarnLimitEnabled",
            "mailBoxWarnLimitValue",
            "mailBoxSendSizelimitEnabled",
            "mailBoxSendSizelimitValue",
            "mailBoxHardSizelimitEnabled",
            "mailBoxHardSizelimitValue",
            "mailBoxAutomaticRemovalEnabled",
            "mailBoxAutomaticRemovalValue",
            "localDeliveryOnly",
            "dropOwnMails"
            );

    var $flagAttrinutes = array("vacationEnabled","mailBoxWarnLimitEnabled","mailBoxSendSizelimitEnabled",
            "mailBoxHardSizelimitEnabled","mailBoxAutomaticRemovalEnabled","localDeliveryOnly","dropOwnMails");

    var $mailAddressSelectDialog = NULL;
    var $vacationTemplates = array();

    var $mailAddress = "";
    var $mailLocation = "";
    var $quotaUsage = "";
    var $quotaSize = "";
    var $alternateAddresses = "";
    var $forwardingAddresses = "";
    var $vacationEnabled = FALSE;
    var $vacationStart = 0;
    var $vacationStop = 0;
    var $vacationMessage = "";
    var $mailBoxWarnLimitEnabled = FALSE;
    var $mailBoxWarnLimitValue = "";
    var $mailBoxSendSizelimitEnabled = FALSE;
    var $mailBoxSendSizelimitValue = "";
    var $mailBoxHardSizelimitEnabled = FALSE;
    var $mailBoxHardSizelimitValue = "";
    var $mailBoxAutomaticRemovalEnabled = FALSE;
    var $mailBoxAutomaticRemovalValue = "";
    var $localDeliveryOnly = FALSE;
    var $dropOwnMails = FALSE;

    function __construct ($config, $dn= NULL)
    {
        plugin::plugin($config,$dn); 

        // Get attributes from parent object 
        foreach(array("uid","cn") as $attr){
            if(isset($this->parent->by_object['group']) && isset($this->parent->by_object['group']->$attr)){
                $this->$attr = &$this->parent->by_object['group']->$attr;
            }elseif(isset($this->attrs[$attr])){
                $this->$attr = $this->attrs[$attr][0];
            }
        }

        // Set vacation start/stop if not set alreasy
        $this->vacationStart = time();
        $this->vacationStop = time() + (14 * 60*60*24);

        // Prepare vacation start/stop time to be initially valid.  
        $this->vacationStart= date('d.m.Y', $this->vacationStart);
        $this->vacationStop= date('d.m.Y', $this->vacationStop);
    }


    function execute()
    {

        // Register plugin execution 
        $display = plugin::execute();

        // Log plugin execution.
        if($this->is_account && !$this->view_logged){
            $this->view_logged = TRUE;
            new log("view","users/".get_class($this),$this->dn);
        }


        /****************
          Account status
         ****************/

        if(isset($_POST['modify_state'])){
            if($this->is_account && $this->acl_is_removeable() && $this->mailMethod->accountRemoveAble()){
                $this->is_account= FALSE;
            }elseif(!$this->is_account && $this->acl_is_createable() && $this->mailMethod->accountCreateable()){
                $this->is_account= TRUE;
            }
        }
        if(!$this->multiple_support_active){
            if (!$this->is_account && $this->parent === NULL){
                $display= "<img alt=\"\" src=\"images/small-error.png\" align=\"middle\">&nbsp;<b>".
                    msgPool::noValidExtension(_("Mail"))."</b>";
                $display.= back_to_main();
                return ($display);
            }
            if ($this->parent !== NULL){
                if ($this->is_account){ 
                    $display= $this->show_disable_header(msgPool::removeFeaturesButton(_("Mail")),msgPool::featuresEnabled(_("Mail")));
                } else {
                    $display= $this->show_enable_header(msgPool::addFeaturesButton(_("Mail")),msgPool::featuresDisabled(_("Mail")));
                    return ($display);
                }
            }
        }

        /****************
          Forward addresses 
         ****************/

        // Display dialog to select a local fowarder 
        if (isset($_POST['addLocalForwardingAddress'])){
            $this->mailAddressSelectDialog=  new mailAddressSelect($this->config, get_userinfo());
            $this->dialog= TRUE;
        }

        // Close dialogs, action was canceled 
        if (isset($_POST['mailAddressSelect_cancel'])){
            $this->mailAddressSelectDialog= FALSE;
            $this->dialog= FALSE;
        }

        // Append selected forwarding addresses now.
        if (isset($_POST['mailAddressSelect_save']) && $this->mailAddressSelectDialog instanceOf mailAddressSelect){
            if($this->acl_is_writeable("gosaMailForwardingAddress")){
                $list = $this->mailAddressSelectDialog->save();
                foreach ($list as $entry){
                    $val = $entry['mail'][0];
                    if (!in_array ($val, $this->gosaMailAlternateAddress) && $val != $this->mail){
                        $this->addForwarder($val);
                        $this->is_modified= TRUE;
                    }
                }
                $this->mailAddressSelectDialog= FALSE;
                $this->dialog= FALSE;
            } else {
                msg_dialog::display(_("Error"), _("Please select an entry!"), ERROR_DIALOG);
            }
        }

        // Display the address selection dialog.
        if($this->mailAddressSelectDialog instanceOf mailAddressSelect){
            $used  = array();
            $used['mail'] = array_values($this->gosaMailAlternateAddress);  
            $used['mail'] = array_merge($used['mail'], array_values($this->gosaMailForwardingAddress));  
            $used['mail'][] = $this->mail;

            // Build up blocklist
            session::set('filterBlacklist', $used);
            return($this->mailAddressSelectDialog->execute());
        }

        // Add manually inserted forwarding address.
        if (isset($_POST['addForwardingAddress'])){
            if ($_POST['forwardingAddressInput'] != ""){
                $address= get_post('forwardingAddressInput');
                $valid= FALSE;
                if (!tests::is_email($address)){
                    if (!tests::is_email($address, TRUE)){
                        if ($this->is_template){
                            $valid= TRUE;
                        } else {
                            msg_dialog::display(_("Error"), msgPool::invalid(_("Mail address"),
                                        "","","your-address@your-domain.com"),ERROR_DIALOG);
                        }
                    }
                } elseif ($address == $this->mail
                        || in_array($address, $this->gosaMailAlternateAddress)) {
                    msg_dialog::display(_("Error"),_("Cannot add primary address to the list of forwarders!") , ERROR_DIALOG);
                } else {
                    $valid= TRUE;
                }
                if ($valid){
                    if($this->acl_is_writeable("gosaMailForwardingAddress")){
                        $this->addForwarder ($address);
                        $this->is_modified= TRUE;
                    }
                }
            }
        }
        if (isset($_POST['deleteForwardingAddress'])){
            $this->delForwarder ($_POST['forwardingAddressList']);
        }


        /****************
          Alternate addresses 
         ****************/

        // Add manually inserted alternate mail address.
        if (isset($_POST['addAlternateAddress'])){
            $valid= FALSE;
            if (!tests::is_email($_POST['alternateAddressInput'])){
                if ($this->is_template){
                    if (!(tests::is_email($_POST['alternateAddressInput'], TRUE))){
                        msg_dialog::display(_("Error"),msgPool::invalid(_("Mail address"),
                                    "","","your-domain@your-domain.com"),ERROR_DIALOG);
                    } else {
                        $valid= TRUE;
                    }
                } else {
                    msg_dialog::display(_("Error"),msgPool::invalid(_("Mail address"),
                                "","","your-domain@your-domain.com"),ERROR_DIALOG);
                }
            } else {
                $valid= TRUE;
            }
            if ($valid && ($user= $this->addAlternate (get_post('alternateAddressInput'))) != ""){
                $ui= get_userinfo();
                $addon= "";
                if ($user[0] == "!") {
                    $addon= sprintf(_("Address is already in use by group '%s'."), mb_substr($user, 1));
                } else {
                    $addon= sprintf(_("Address is already in use by user '%s'."), $user);
                }
                msg_dialog::display(_("Error"), msgPool::duplicated(_("Mail address"))."<br><br><i>".
                        "$addon</i>", ERROR_DIALOG);
            }
        }

        // Remove alternate mail address.
        if (isset($_POST['deleteAlternateAddress']) && isset($_POST['alternateAddressList'])){
            $this->delAlternate ($_POST['alternateAddressList']);
        }


        /****************
          SMARTY- Assign smarty variables 
         ****************/

        $smarty = get_smarty();
        foreach($this->attributes as $attr){
            $smarty->assign($attr, $this->$attr);
        }

        $plInfo = $this->plInfo();
        foreach($plInfo['plProvidedAcls'] as $acl => $name){
            $smarty->assign($acl."ACL", $this->getacl($acl));
        }

        $smarty->assign("mailLocations", array("tester"));
        if (count($this->vacationTemplates)){
            $smarty->assign("displayTemplateSelector", "true");
            $smarty->assign("vacationTemplate", set_post($this->vacationTemplate));
            $smarty->assign("vacationTemplates", set_post($this->vacationTemplates));
            $smarty->assign("template", set_post(get_post('vacation_template')));
        } else {
            $smarty->assign("displayTemplateSelector", "false");
        }

        return($display.$smarty->fetch(get_template_path("generic.tpl",TRUE,dirname(__FILE__))));
    }



    /* Save data to object */
    function save_object()
    {
        if(isset($_POST['groupwarePluginPosted'])){
            foreach($this->attributes as $attr){
                if(isset($_POST[$attr])){
                    $this->$attr = get_post($attr);
                }
            }

            foreach($this->flagAttrinutes as $attr){
                $this->$attr = isset($_POST[$attr]);
            }
        }
    }


    /*! \brief  Parse vacation templates and build up an array
      containing 'filename' => 'description'. 
      Used to fill vacation dropdown box.
      @return Array   All useable vacation templates.
     */ 
    function get_vacation_templates()
    {
        $vct = array();
        if ($this->config->get_cfg_value("core","vacationTemplateDirectory") != ""){
            $dir= $this->config->get_cfg_value("core","vacationTemplateDirectory");
            if (is_dir($dir) && is_readable($dir)){
                $dh = opendir($dir);
                while($file = readdir($dh)){
                    $description= "";
                    if (is_file($dir."/".$file)){
                        $fh = fopen($dir."/".$file, "r");
                        $line= fgets($fh, 256);
                        if (!preg_match('/^DESC:/', $line)){
                            msg_dialog::display(_("Configuration error"), sprintf(_("No DESC tag in vacation template '%s'!"), $file), ERROR_DIALOG);
                        }else{
                            $description= trim(preg_replace('/^DESC:\s*/', '', $line));
                        }
                        fclose ($fh);
                    }
                    if ($description != ""){
                        $vct["$dir/$file"]= $description;
                    }
                }
                closedir($dh);
            }
        }
        return($vct); 
    }


    /*! \brief  Adds the given mail address to the list of mail forwarders 
     */ 
    function addForwarder($address)
    {
        if(empty($address)) return;
        if($this->acl_is_writeable("gosaMailForwardingAddress")){
            $this->gosaMailForwardingAddress[]= $address;
            $this->gosaMailForwardingAddress= array_unique ($this->gosaMailForwardingAddress);
            sort ($this->gosaMailForwardingAddress);
            reset ($this->gosaMailForwardingAddress);
            $this->is_modified= TRUE;
        }else{
            msg_dialog::display(_("Permission error"), _("You have no permission to modify these addresses!"), ERROR_DIALOG);
        }
    }


    /*! \brief  Removes the given mail address from the list of mail forwarders 
     */ 
    function delForwarder($addresses)
    {
        if($this->acl_is_writeable("gosaMailForwardingAddress")){
            $this->gosaMailForwardingAddress= array_remove_entries ($addresses, $this->gosaMailForwardingAddress);
            $this->is_modified= TRUE;
        }else{
            msg_dialog::display(_("Permission error"), _("You have no permission to modify these addresses!"), ERROR_DIALOG);
        }
    }


    /*! \brief  Add given mail address to the list of alternate adresses ,
      .          check if this mal address is used, skip adding in this case 
     */ 
    function addAlternate($address)
    {
        if(empty($address)) return;
        if($this->acl_is_writeable("gosaMailAlternateAddress")){
            $ldap= $this->config->get_ldap_link();
            $address= strtolower($address);

            /* Is this address already assigned in LDAP? */
            $ldap->cd ($this->config->current['BASE']);
            $ldap->search ("(&(!(objectClass=gosaUserTemplate))(objectClass=gosaMailAccount)(|(mail=$address)".
                    "(alias=$address)(gosaMailAlternateAddress=$address)))", array("uid", "cn"));
            if ($ldap->count() > 0){
                $attrs= $ldap->fetch ();
                if (!isset($attrs["uid"])) {
                    return ("!".$attrs["cn"][0]);
                }
                return ($attrs["uid"][0]);
            }
            if (!in_array($address, $this->gosaMailAlternateAddress)){
                $this->gosaMailAlternateAddress[]= $address;
                $this->is_modified= TRUE;
            }
            sort ($this->gosaMailAlternateAddress);
            reset ($this->gosaMailAlternateAddress);
            return ("");
        }else{
            msg_dialog::display(_("Permission error"), _("You have no permission to modify these addresses!"), ERROR_DIALOG);
        }
    }


    /*! \brief  Removes the given mail address from the alternate addresses list 
     */ 
    function delAlternate($addresses)
    {
        if($this->acl_is_writeable("gosaMailAlternateAddress")){
            $this->gosaMailAlternateAddress= array_remove_entries ($addresses,$this->gosaMailAlternateAddress);
            $this->is_modified= TRUE;
        }else{
            msg_dialog::display(_("Permission error"), _("You have no permission to modify these addresses!"), ERROR_DIALOG);
        }
    }


    /*! \brief  Prepare importet vacation string. \
      .         Replace placeholder like %givenName a.s.o.
      @param  string  Vacation string
      @return string  Completed vacation string
     */
    private function prepare_vacation_template($contents)
    {
        /* Replace attributes */
        $attrs = array();
        $obj   = NULL;
        if(isset($this->parent->by_object['user'])){
            $attrs  = $this->parent->by_object['user']->attributes;
            $obj    = $this->parent->by_object['user'];
        }else{
            $obj    = new user($this->config,$this->dn);
            $attrs  = $obj->attributes;
        }
        if($obj){

            /* Replace vacation start and end time */
            if($this->mailMethod->vacationRangeEnabled()){
                if(preg_match("/%start/",$contents)){
                    $contents = preg_replace("/%start/",$this->vacationStart,$contents);
                }
                if(preg_match("/%end/",$contents)){
                    $contents = preg_replace("/%end/",$this->vacationStop,$contents);
                }
            }else{
                if(preg_match("/%start/",$contents)){
                    $contents = preg_replace("/%start/", _("unknown"),$contents);
                }
                if(preg_match("/%end/",$contents)){
                    $contents = preg_replace("/%end/", _("unknown"), $contents);
                }
            }

            foreach ($attrs as $val){

                // We can only replace strings here
                if(!is_string($obj->$val)) continue;

                if(preg_match("/dateOfBirth/",$val)){
                    if($obj->use_dob){
                        $contents= preg_replace("/%$val/",date("Y-d-m",$obj->dateOfBirth),$contents);
                    }
                }else {
                    $contents= preg_replace("/%$val/",
                            $obj->$val, $contents);
                }

            }
        }
        $contents = ltrim(preg_replace("/^DESC:.*$/m","",$contents),"\n ");
        return($contents);
    }


    function remove_from_parent()
    {
        /* Cancel if there's nothing to do here */
        if (!$this->initially_was_account){
            return;
        }

    }


    function save()
    {
    }


    /*! \brief  Check given values 
     */
    function check()
    {
        if(!$this->is_account){
            return(array());
        }

        $ldap= $this->config->get_ldap_link();

        /* Call common method to give check the hook */
        $message= plugin::check();

        if(empty($this->gosaMailServer)){
            $message[]= msgPool::noserver(_("Mail"));
        }

        /* Mail address checks */
        $mail = $this->mail;
        if(!(!$this->mailMethod->isModifyableMail() && $this->initially_was_account)){

            if($this->mailMethod->domainSelectionEnabled()){
                $mail.= "@".$this->mailDomainPart;
            }

            if (empty($mail)){
                $message[]= msgPool::required(_("Primary address"));
            }
            if ($this->is_template){
                if (!tests::is_email($mail, TRUE)){
                    $message[]= msgPool::invalid(_("Mail address"),"","","%givenName.%sn@your-domain.com");
                }
            } else {
                if (!tests::is_email($mail)){
                    $message[]= msgPool::invalid(_("Mail address"),"","","your-address@your-domain.com");
                }
            }

            /* Check if this mail address is already in use */
            $ldap->cd($this->config->current['BASE']);
            $filter = "(&(!(objectClass=gosaUserTemplate))(!(uid=".$this->uid."))".
                "(objectClass=gosaMailAccount)".
                "(|(mail=".$mail.")(alias=".$mail.")(gosaMailAlternateAddress=".$mail.")))";
            $ldap->search($filter,array("uid", "cn"));
            if ($ldap->count() != 0){
                $entry= $ldap->fetch();
                $addon= "";
                if (!isset($entry['uid'])) {
                    $addon= sprintf(_("Address is already in use by group '%s'."), $entry['cn'][0]);
                } else {
                    $addon= sprintf(_("Address is already in use by user '%s'."), $entry['uid'][0]);
                }
                $message[]= msgPool::duplicated(_("Mail address"))."<br><br><i>$addon</i>";
            }
        }


        /* Check quota */
        if ($this->gosaMailQuota != '' && $this->acl_is_writeable("gosaMailQuota")){
            if (!is_numeric($this->gosaMailQuota)) {
                $message[]= msgPool::invalid(_("Quota size"),$this->gosaMailQuota,"/^[0-9]*/");
            } else {
                $this->gosaMailQuota= (int) $this->gosaMailQuota;
            }
        }

        /* Check rejectsize for integer */
        if ($this->gosaMailMaxSize != '' && $this->acl_is_writeable("gosaMailMaxSize")){
            if (!is_numeric($this->gosaMailMaxSize)){
                $message[]= msgPool::invalid(_("Mail reject size"),$this->gosaMailMaxSize,"/^[0-9]*/");
            } else {
                $this->gosaMailMaxSize= (int) $this->gosaMailMaxSize;
            }
        }

        /* Need gosaMailMaxSize if use_mailsize_limit is checked */
        if (is_integer(strpos($this->gosaMailDeliveryMode, "R")) && $this->gosaMailMaxSize == ""){
            $message[]= msgPool::required(_("Mail reject size"));
        }

        if((preg_match("/S/", $this->gosaMailDeliveryMode))&&(empty($this->gosaSpamMailbox))) {
            $message[]= msgPool::required(_("Spam folder"));
        }

        if ($this->mailMethod->vacationRangeEnabled() && preg_match('/V/', $this->gosaMailDeliveryMode)){ 

            /* Check date strings */
            $state= true;
            if ($this->vacationStart == "" || !tests::is_date($this->vacationStart)) {
                $message[]= msgPool::invalid(_("from"),$this->vacationStart);
                $state= false;
            }
            if ($this->vacationStart == "" || !tests::is_date($this->vacationStop)) {
                $message[]= msgPool::invalid(_("to"),$this->vacationStop);
                $state= false;
            }

#TODO: take care of date format
            if ($state) {
                list($day, $month, $year)= explode('.', $this->vacationStart);
                $start= mktime(0,0,0,$month, $day, $year);
                list($day, $month, $year)= explode('.', $this->vacationStop);
                $stop= mktime(0,0,0,$month, $day, $year);
                if($start > $stop){
                    $message[]= msgPool::invalid(_("Vacation interval"));
                }
            }
        }
        return($message);
    }


    /*! \brief  Adapt from template, using 'dn' 
     */
    function adapt_from_template($dn, $skip= array())
    {
        plugin::adapt_from_template($dn, $skip);

    }


    /*! \brief  ACL settings 
     */
    static function plInfo()
    {
        return (array(
                    "plShortName"     => _("Mail"),
                    "plDescription"   => _("Mail settings"),
                    "plSelfModify"    => TRUE,
                    "plDepends"       => array("user"),                     // This plugin depends on
                    "plPriority"      => 4,                                 // Position in tabs
                    "plSection"     => array("personal" => _("My account")),
                    "plCategory"    => array("users"),
                    "plOptions"       => array(),
                    "plProvidedAcls"  => array(
                        "mailAddress"                   => _("Mail address"),
                        "mailLocation"                  => _("Mail location"),
                        "quotaUsage"                    => _("Quota usage"),
                        "mailFilter"                    => _("Mail filter"),
                        "quotaSize"                     => _("Quota size"),
                        "alternateAddresses"            => _("Alternate mail addresses"),
                        "forwardingAddresses"           => _("Forwarding mail addresses"),
                        "vacationEnabled"               => _("Vaction switch"),
                        "vacationStart"                 => _("Vacation start time"),
                        "vacationStop"                  => _("Vacation stop time"),
                        "vacationMessage"               => _("Vacation message"),
                        "mailBoxWarnLimit"              => _("Warn sizelimit"),
                        "mailBoxSendSizelimit"          => _("Send sizelimit"),
                        "mailBoxHardSizelimit"          => _("Hard sizelimit"),
                        "mailBoxAutomaticRemoval"       => _("Automatic mail removal"),
                        "localDeliveryOnly"             => _("Local delivery only"),
                        "dropOwnMails"                  => _("Drop own mails")
                        )
                    ));
    }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
