<?php

class groupware extends plugin
{
    var $plHeadline     = "Mail";
    var $plDescription  = "GOsa mail extension.";
    var $view_logged = FALSE;

    var $accountInitialized = FALSE;
    var $rpcError = FALSE;
    var $rpcErrorMessage = "";

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

    var $enabledFeatures = array(
            "quotaUsage"                => TRUE,
            "quotaSize"                 => TRUE,
            "mailFilter"                => TRUE,
            "alternateAddresses"        => TRUE,
            "forwardingAddresses"       => TRUE,
            "vacationMessage"           => TRUE,
            "mailBoxWarnLimit"          => TRUE,
            "mailBoxSendSizelimit"      => TRUE,
            "mailBoxHardSizelimit"      => TRUE,
            "mailBoxAutomaticRemoval"   => TRUE,
            "localDeliveryOnly"         => TRUE,
            "dropOwnMails"              => TRUE);

    var $flagAttributes = array("vacationEnabled","mailBoxWarnLimitEnabled","mailBoxSendSizelimitEnabled",
            "mailBoxHardSizelimitEnabled","mailBoxAutomaticRemovalEnabled","localDeliveryOnly","dropOwnMails");

    var $mailAddressSelectDialog = NULL;
    var $filterManager = NULL;
    var $filterRules = array();
    var $vacationTemplates = array();

    var $mailAddress = "";
    var $mailLocation = "";
    var $quotaUsage = 0;
    var $quotaSize = 0;
    var $alternateAddresses = array();
    var $forwardingAddresses = array();
    var $vacationEnabled = FALSE;
    var $vacationStart = 0;
    var $vacationStop = 0;
    var $vacationMessage = "";
    var $mailBoxWarnLimitEnabled = FALSE;
    var $mailBoxWarnLimitValue = 100;
    var $mailBoxSendSizelimitEnabled = FALSE;
    var $mailBoxSendSizelimitValue = 100;
    var $mailBoxHardSizelimitEnabled = FALSE;
    var $mailBoxHardSizelimitValue = 100;
    var $mailBoxAutomaticRemovalEnabled = FALSE;
    var $mailBoxAutomaticRemovalValue = 100;
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

        // Initialize the plugin using rpc.
        $this->init();
    }


    /*! \brief  Try to execute a function on the gosa backend using json-rpc.
     *          This method also takes care about errors and sets the required
     *           class members, such as rpcError and rpcErrorMessage. 
     *  @param  String  function    The name of the function to call.
     *  @param  Mixed   args[0-n]   The parameter to use.
     *  @return Mixed   The result of the function call on success else NULL.
     */
    function rpcExec($function)
    {
        $params = func_get_args();
        unset($params[0]);
        $rpc = $this->config->getRpcHandle();
        $res = call_user_func_array(array($rpc,$function),array_values($params));
        $this->rpcError = !$rpc->success();
        if($this->rpcError){
            $this->rpcErrorMessage = $rpc->getError();
            return(NULL);
        }
        return($res);
    }


    /*! \brief  Try initialize the groupware account.
     *          This method fetches all required information to manage the
     *           account using the GOsa gui.
     */
    function init()
    {
        // Get rpc handle to fetch account info and feature availability.
        $status = $this->rpcExec('gwAcctExists', $this->uid);
        if($status !== NULL){
            $this->initially_was_account = $this->is_account = $status;
            $this->accountInitialized = TRUE;
        }

        // Set vacation start/stop if not set alreasy
        $this->vacationStart = time();
        $this->vacationStop = time() + (14 * 60*60*24);

        // Prepare vacation start/stop time to be initially valid.  
        $this->vacationStart= date('d.m.Y', $this->vacationStart);
        $this->vacationStop= date('d.m.Y', $this->vacationStop);
    }


    /*! \brief  Generates the HTML user interface for the groupware plugin
     *           and take of several ui actions like adding or removing 
     *           forward addresses, filters and the account itself.
     */
    function execute()
    {
        // Register plugin execution 
        $display = plugin::execute();

        // Log plugin execution.
        if($this->is_account && !$this->view_logged){
            $this->view_logged = TRUE;
            new log("view","users/".get_class($this),$this->dn);
        }

        // Check if we were able to initialize the account already.
        if(!$this->accountInitialized){
            $this->init();
            if(!$this->accountInitialized){
                $smarty = get_smarty();
                $smarty->assign("initFailed", !$this->accountInitialized);
                return($smarty->fetch(get_template_path("generic.tpl",TRUE,dirname(__FILE__))));
            }
        }

        // Check if we were able to initialize the account already.
        if($this->rpcError){
            $smarty = get_smarty();
            $smarty->assign("initFailed", !$this->accountInitialized);
            $smarty->assign("rpcError", $this->rpcError);
            return($smarty->fetch(get_template_path("generic.tpl",TRUE,dirname(__FILE__))));
        }

        /****************
          Filter editor
         ****************/

        if(isset($_POST['filterManager_cancel'])) $this->filterManager = NULL;
        if(isset($_POST['filterManager_ok'])){
            $this->filterManager->save_object();
            $msgs = $this->filterManager->check();
            if(count($msgs)){
                msg_dialog::displayChecks($msgs);
            }else{
                $this->filterRules = $this->filterManager->save();
                $this->filterManager = NULL;
            }
        }
        if(isset($_POST['configureFilter'])){
            $this->filterManager = new filterManager($this->config, $this,$this->filterRules);
            $this->filterManager->acl_base = $this->acl_base;
            $this->filterManager->acl_category = $this->acl_category;
        }
        $this->dialog = FALSE;
        if($this->filterManager instanceOf filterManager){
            $this->filterManager->save_object();
            $this->dialog = TRUE;
            return($this->filterManager->execute());
        }


        /****************
          Account status
         ****************/

        if(isset($_POST['modify_state'])){
            if($this->is_account && $this->acl_is_removeable()){
                $this->is_account= FALSE;
            }elseif(!$this->is_account && $this->acl_is_createable()){
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
            if($this->acl_is_writeable("forwardingAddresses")){
                $list = $this->mailAddressSelectDialog->save();
                foreach ($list as $entry){
                    $val = $entry['mail'][0];
                    if (!in_array ($val, $this->alternateAddresses) && $val != $this->mailAddress){
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
            $used['mail'] = array_values($this->alternateAddresses);  
            $used['mail'] = array_merge($used['mail'], array_values($this->forwardingAddresses));  
            $used['mail'][] = $this->mailAddress;

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
                } elseif ($address == $this->mailAddress || in_array($address, $this->alternateAddresses)) {
                    msg_dialog::display(_("Error"),_("Cannot add primary address to the list of forwarders!") , ERROR_DIALOG);
                } else {
                    $valid= TRUE;
                }
                if ($valid){
                    if($this->acl_is_writeable("forwardingAddresses")){
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
        foreach($this->enabledFeatures as $feature => $state){
            $smarty->assign($feature."_isActive", $state);
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

        $smarty->assign("initFailed", !$this->accountInitialized);
        $smarty->assign("rpcError", $this->rpcError);
        $smarty->assign("rpcErrorMessage", $this->rpcErrorMessage);
        return($display.$smarty->fetch(get_template_path("generic.tpl",TRUE,dirname(__FILE__))));
    }



    /*! \brief      This method handles potential _POST and _GET values.
     *              It captures modifcations from the ui, like changing 
     *               the mailAddress.
     *              This method respects the attribute permissions.
     */    
    function save_object()
    {
        if(isset($_POST['groupwarePluginPosted'])){

            // We ran into a communication error with the backend. 
            // Try a simple communication operation with the backend 
            //  again and let us see if it works.
            if(isset($_POST['retry'])){
                $this->rpcExec('getCapabilities');
            }

            // Get ui modifications and store them in the class.
            foreach($this->attributes as $attr){
                if(isset($_POST[$attr]) && $this->acl_is_writeable($attr)){
                    $this->$attr = get_post($attr);
                }
            }

            echo 'ACL checks missing!';

            foreach($this->flagAttributes as $attr){
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
        if($this->acl_is_writeable("forwardingAddresses")){
            $this->forwardingAddresses[]= $address;
            $this->forwardingAddresses= array_unique ($this->forwardingAddresses);
            sort ($this->forwardingAddresses);
            reset ($this->forwardingAddresses);
            $this->is_modified= TRUE;
        }else{
            msg_dialog::display(_("Permission error"), _("You have no permission to modify these addresses!"), ERROR_DIALOG);
        }
    }


    /*! \brief  Removes the given mail address from the list of mail forwarders 
     */ 
    function delForwarder($addresses)
    {
        if($this->acl_is_writeable("forwardingAddresses")){
            $this->forwardingAddresses= array_remove_entries ($addresses, $this->forwardingAddresses);
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
        if($this->acl_is_writeable("alternateAddresses")){
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
            if (!in_array($address, $this->alternateAddresses)){
                $this->alternateAddresses[]= $address;
                $this->is_modified= TRUE;
            }
            sort ($this->alternateAddresses);
            reset ($this->alternateAddresses);
            return ("");
        }else{
            msg_dialog::display(_("Permission error"), _("You have no permission to modify these addresses!"), ERROR_DIALOG);
        }
    }


    /*! \brief  Removes the given mail address from the alternate addresses list 
     */ 
    function delAlternate($addresses)
    {
        if($this->acl_is_writeable("alternateAddresses")){
            $this->alternateAddresses= array_remove_entries ($addresses,$this->alternateAddresses);
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
            if($this->enabledFeatures['vacationMessage']){
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
        // Get rpc handle to remove the account
        if($this->initially_was_account){
            if($this->rpcExec('gwAcctDel', $this->uid) === NULL){
                msg_dialog::display(_("Error"), _("Groupware account removal failed!"), ERROR_DIALOG);
            }
        }
    }


    function save()
    {
        // Get rpc handle to create or update the account
        if(!$this->initially_was_account){
            if($this->rpcExec('gwAcctAdd', $this->uid, $this->mailAddress) === NULL){
                msg_dialog::display(_("Error"), _("Groupware account creation failed!"), ERROR_DIALOG);
            }
        }
    }


    /*! \brief  Check given values 
     */
    function check()
    {
        $messages = plugin::check();
        return($messages);
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
                    "plShortName"     => _("Groupware"),
                    "plDescription"   => _("Groupware settings"),
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
