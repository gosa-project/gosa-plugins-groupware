<?php
/*
 * This code is part of GOsa (https://gosa.gonicus.de)
 * Copyright (C) 2008 Cajus Pollmeier
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

# Access constants
if(!defined("GROUPWARE_LOOKUP")){
    define ("GROUPWARE_LOOKUP", 1);
    define ("GROUPWARE_READ", 2);
    define ("GROUPWARE_STATUS", 4);
    define ("GROUPWARE_WRITE", 8);
    define ("GROUPWARE_INSERT", 16);
    define ("GROUPWARE_POST", 32);
    define ("GROUPWARE_CREATE", 64);
    define ("GROUPWARE_DELETE", 128);
    define ("GROUPWARE_ADMINISTRATE", 256);


    define ("GROUPWARE_RIGHTS_NONE", 0);
    define ("RIGHTS_GROUPWARE_READ",  GROUPWARE_LOOKUP | GROUPWARE_READ | GROUPWARE_STATUS);
    define ("RIGHTS_GROUPWARE_POST", RIGHTS_GROUPWARE_READ | GROUPWARE_POST);
    define ("GROUPWARE_RIGHTS_APPEND", RIGHTS_GROUPWARE_POST | GROUPWARE_INSERT);
    define ("RIGHTS_GROUPWARE_WRITE", GROUPWARE_RIGHTS_APPEND | GROUPWARE_WRITE | GROUPWARE_CREATE | GROUPWARE_DELETE);
    define ("GROUPWARE_RIGHTS_ALL", RIGHTS_GROUPWARE_WRITE | GROUPWARE_ADMINISTRATE);
}




class Groupware extends plugin
{
    // Folder related attributes, will be moved into a speperate widget late on
    var $currentSelectedFolder = "";     

    var $plHeadline     = "Mail";
    var $plDescription  = "GOsa mail extension.";
    var $view_logged = FALSE;

    var $FolderWidget = NULL;

    var $accountInitialized = FALSE;
    var $rpcError = FALSE;
    var $rpcErrorMessage = "";

    var $attributes = array(
            "mailAddress", "mailLocation", "quotaUsage", "quotaSize", "alternateAddresses",
            "forwardingAddresses", "vacationEnabled", "vacationStart", "vacationStop",
            "vacationMessage", "mailBoxWarnLimitEnabled", "mailBoxWarnLimitValue",
            "mailBoxSendSizelimitEnabled", "mailBoxSendSizelimitValue", "mailBoxHardSizelimitEnabled",
            "mailBoxHardSizelimitValue", "mailBoxAutomaticRemovalEnabled", "mailBoxAutomaticRemovalValue",
            "localDeliveryOnly", "dropOwnMails", "mailFolder");


    var $enabledFeatures = array();

    var $flagAttributes = array("vacationEnabled","mailBoxWarnLimitEnabled","mailBoxSendSizelimitEnabled",
            "mailBoxHardSizelimitEnabled","mailBoxAutomaticRemovalEnabled","localDeliveryOnly","dropOwnMails");

    var $mailAddressSelectDialog = NULL;
    var $filterManager = NULL;
    var $filterRules = array();
    var $vacationTemplates = array(); 

	var $fileBrowser = NULL;

	
    //the dropdown
    var $mailLocations = array();

    var $mailAddress = "";
    var $mailLocation = "";
    var $mailFolder = array();
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
    var $groupwareDao = null;

    // Used in the folder editor as the starting folder, usually user/username
    var $folderPrefix = "";

    /*! \brief      
     */
    function __construct ($config, $dn= NULL)
    {
        plugin::plugin($config,$dn); 

        // Get attributes from parent object 
        if(isset($this->attrs['uid'])){
            $this->uid = $this->attrs['uid'][0];
        }

        // Set initial folder prefix
        $this->folderPrefix = "user/{$this->uid}";

		// Initialize file browser list
		$this->fileBrowser= new sortableListing();
		$this->fileBrowser->setDeleteable(TRUE);
		$this->fileBrowser->setEditable(TRUE);
		$this->fileBrowser->setColspecs(array('*'));
		$this->fileBrowser->setWidth("100%");
		$this->fileBrowser->setHeight("150px");
		$this->fileBrowser->setAcl("rwcdm");
		$this->fileBrowser->update();
		$this->fileBrowser->sortingEnabled(FALSE);

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
            $this->rpcErrorMessage = $rpc->get_error();
            return(NULL);
        }
        return($res);
    }


    /*! \brief     TODO: comment
     */
    public function isFeatureEnabled($featureName)
    {
        return(isset($this->enabledFeatures[$featureName]) && $this->enabledFeatures[$featureName]);
    }


    /*! \brief  Try initialize the groupware account.
     *          This method fetches all required information to manage the
     *           account using the GOsa gui.
     */
    function init()
    {
        // Detect feature availability and enable/disable services correspondingly.
        $this->groupwareDao = new GroupwareDao($this);

        // Feature names with a list of groupware functions that must be available.
        $features = array();
        $featureReq = array(
                "primaryMail" 				=> array(
                    'acctGetPrimaryMailAddress'),
                "quotaUsage"                => array(
                    'acctGetQuota'),
                "quotaSize"                 => array(
                    'acctSetQuota','acctGetQuota'),
                "mailLocations"				=> array(
                    'getMailboxLocations'),
                "mailFilter"                => array("_off",
                    'acctDelFilter','acctGetFilters','acctSetFilters','acctSetFilters'),
                "alternateAddresses"        => array(
                    'acctDelAlternateMailAddress','acctSetAlternateMailAddresses',
                    'acctAddAlternateMailAddress','acctGetAlternateMailAddresses'),
                "forwardingAddresses"       => array(
                    'acctAddMailForwardAddress','acctDelMailForwardAddress',
                    'acctGetMailForwardAddresses','acctSetMailForwardAddresses'),
                "vacationMessage"           => array("_off",
                    'acctDelFilter','acctGetFilters','acctSetFilters','acctSetFilters'),
                "mailBoxWarnLimit"          => array(
                    'acctSetQuota','acctGetQuota'),
                "mailBoxSendSizelimit"      => array(
                        'acctSetQuota','acctGetQuota'),
                "mailBoxHardSizelimit"      => array(
                        'acctSetQuota','acctGetQuota'),
                "mailBoxAutomaticRemoval"   => array("_off",
                        'acctSetQuota','acctGetQuota'),
                "localDeliveryOnly"         => array( "_off",
                        'acctDelFilter','acctGetFilters','acctSetFilters','acctSetFilters'),
                "dropOwnMails"              => array( "_off",
                        'acctDelFilter','acctGetFilters','acctSetFilters','acctSetFilters'),
				"mailFolder"				=> array(
						'folderList','folderDel','folderAdd')
				);

        // Check if all required methods can be called! 
		foreach($featureReq as $name => $requires){
			$active = TRUE;
			foreach($requires as $methodName){
				$active &= $this->groupwareDao->gwFeatureAvailable($methodName);
			}
			$this->enabledFeatures[$name] = $active;
		}

        // If this is a new user, then do not try to check account status 
        //  the account cannot exists.			
        $this->mailLocations = $this->groupwareDao->get('mailLocations', array());
        if($this->dn == "new"){
            $this->initially_was_account = $this->is_account = FALSE;
            $this->accountInitialized = TRUE;
        }else{

            // Get rpc handle to fetch account info and feature availability.
            $status = $this->rpcExec('gwAcctExists', $this->uid);
            if($status !== NULL){
                $this->initially_was_account = $this->is_account = $status;
                if($this->is_account){
                    $response = $this->groupwareDao->getComprehensiverUser($this->uid);
                    $this->mapComprehensiveUserData($response);
                }
                $this->accountInitialized = TRUE;
            }
        }

        // Set vacation start/stop if not set alreasy
        $this->vacationStart = time();
        $this->vacationStop = time() + (14 * 60*60*24);

        // Prepare vacation start/stop time to be initially valid.  
        $this->vacationStart= date('d.m.Y', $this->vacationStart);
        $this->vacationStop= date('d.m.Y', $this->vacationStop);
    }


    function getPermissions()
    {
        $ret[GROUPWARE_RIGHTS_NONE] = _("None");
        $ret[RIGHTS_GROUPWARE_READ] = _("Read");
        $ret[RIGHTS_GROUPWARE_POST] = _("Post");
        $ret[GROUPWARE_RIGHTS_APPEND] = _("Append");
        $ret[RIGHTS_GROUPWARE_WRITE] = _("Write");
        $ret[GROUPWARE_RIGHTS_ALL] = _("All");
        return($ret);
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
            $this->filterManager = new FilterManager($this->config, $this,$this->filterRules);
            $this->filterManager->acl_base = $this->acl_base;
            $this->filterManager->acl_category = $this->acl_category;
        }
        $this->dialog = FALSE;
        if($this->filterManager instanceOf FilterManager){
            $this->filterManager->save_object();
            $this->dialog = TRUE;
            return($this->filterManager->execute());
        }


        /****************
          Folder editor
         ****************/

        if(isset($_POST['FolderWidget_cancel'])) $this->FolderWidget = NULL;
        if(isset($_POST['FolderWidget_ok'])){
            $this->FolderWidget->save_object();
            $msgs = $this->FolderWidget->check();
            if(count($msgs)){
                msg_dialog::displayChecks($msgs);
            }else{
                $this->mailFolder = $this->FolderWidget->save();
                $this->FolderWidget = NULL;
            }
        }
        if(isset($_POST['configureFolder'])){
            $this->FolderWidget = new FolderWidget($this->config,$this->mailFolder, $this->folderPrefix);
            $this->FolderWidget->setPermissions($this->getPermissions());
            $this->FolderWidget->acl_base = $this->acl_base;
            $this->FolderWidget->acl_category = $this->acl_category;
        }
        $this->dialog = FALSE;
        if($this->FolderWidget instanceOf FolderWidget){
            $this->FolderWidget->save_object();
            $this->dialog = TRUE;
            return($this->FolderWidget->execute());
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
       
        // Build up folder list 
        $data = $lData = array();
        foreach($this->mailFolder as $folderPath => $entries){
            $data[$folderPath] = $entries;
            $name = $entries['name'];
            $cnt = count($entries['acls']);
            if($this->currentSelectedFolder == $folderPath){
                $name = "<b>{$name}</b>";
            }

            $status = (isset($entries['status']))? $entries['status'] : '';        
            $prefix = preg_replace("/^([^\/]*).*$/","\\1", $folderPath);
            $name = str_pad($name, (substr_count($folderPath, '/')*3-3) + strlen($name),'../', STR_PAD_LEFT);
            $lData[$folderPath]= array('data' => array($name,$prefix,$status,$cnt));
        }    
        $this->fileBrowser->setListData($data, $lData) ;
        $this->fileBrowser->update();

        
        $smarty->assign("currentSelectedFolder", $this->currentSelectedFolder);
        $folderEntries = array();
        if(isset($this->mailFolder[$this->currentSelectedFolder])) {
            $folderEntries = $this->mailFolder[$this->currentSelectedFolder];
        }
		$smarty->assign("folderEntries", $folderEntries);

		$smarty->assign("fileBrowser", $this->fileBrowser->render());
        $smarty->assign("mailLocations", $this->mailLocations);
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
            // again and let us see if it works.
            if(isset($_POST['retry'])){
                $this->rpcExec('gwGetCapabilities');
            }

            // Get ui modifications and store them in the class.
            $testAttrs = array("mailAddress","mailLocation","quotaUsage","quotaSize",
                    "alternateAddresses","forwardingAddresses","vacationEnabled","vacationStart",
                    "vacationStop","vacationMessage");
            foreach($testAttrs as $attr){
                if(isset($_POST[$attr]) && $this->acl_is_writeable($attr)){
                    $this->$attr = get_post($attr);
                }
            }

            // Detect checkbox states 
            $checkAttrs = array("mailBoxWarnLimit","mailBoxSendSizelimit",
                    "mailBoxHardSizelimit","mailBoxAutomaticRemoval");
            foreach($checkAttrs as $boxname){
                if($this->acl_is_writeable($boxname)){
                    $v = $boxname."Value"; 
                    $e = $boxname."Enabled"; 
                    $this->$e = isset($_POST[$e]);
                    if($this->$e){
                        $this->$v = get_post($v);
                    }
                }
            }

            // Get posted flag changes 
            $flagAttrs = array("localDeliveryOnly","dropOwnMails");
            foreach($flagAttrs as $attr){
                $this->$attr = isset($_POST[$attr]);
            }
        }
    }

    /*! \brief  Parse vacation templates and build up an array
     *	containing 'filename' => 'description'. 
     *	Used to fill vacation dropdown box.
     *	@return Array   All useable vacation templates.
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
     *           check if this mal address is used, skip adding in this case 
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
     *           Replace placeholder like %givenName a.s.o.
     * @param  string  Vacation string
     * @return string  Completed vacation string
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


    /*! \brief     Remove the account form the groupware server completely. 
     */
    function remove_from_parent()
    {
        // Get rpc handle to remove the account
        if($this->initially_was_account){
            if($this->rpcExec('gwAcctDel', $this->uid) === NULL){
                msg_dialog::display(_("Error"), _("Groupware account removal failed!"), ERROR_DIALOG);
            }
        }
    }


    /* \brief   Persists the values of this plugin to the groupware server.
     *          Save method is called on "apply" and "ok" in the Gosa Frontend.
     *          All other actions will update values in the form or 
     *           cancel and therefore discard the changes so far.
     */
    function save()
    {
        // Get rpc handle to create or update the account
        if(!$this->initially_was_account){
            $this->rpcExec('gwAcctAdd', $this->uid, $this->mailAddress);
            if($this->rpcError){
                msg_dialog::display(_("Error"), _("Groupware account creation failed!"), ERROR_DIALOG);
                return;
            }
        }

        

        $this->groupwareDao->saveFoldersAndAcls($this->uid, $this->mailFolder);

        // Save the primary Email Address.
        if(!empty($this->mailAddress)){
            $this->groupwareDao->save("primaryMail", $this->uid, $this->mailAddress);
        }

        // Save alternateAddresses and forwarding. 
        if(!empty($this->alternateAddresses) 
        		&& isset($this->alternateAddresses) 
        		&& is_array($this->alternateAddresses)){
			$this->groupwareDao->save("alternateAddresses", $this->uid, array($this->alternateAddresses));

        }
        if(!empty($this->forwardingAddresses) 
        		&& isset($this->forwardingAddresses) 
        		&& is_array($this->forwardingAddresses)){
				
				// Only the first Address will be set as forward Address.
				$fw = array();
				$fw[$this->forwardingAddresses[0]] = true;
				$this->groupwareDao->save("forwardingAddresses", $this->uid, array($fw));
        }

        $quota = array(	"warn_limit" => $this->mailBoxWarnLimitValue,
                "send_limit" => $this->mailBoxSendSizelimitValue,
                "hard_limit" => $this->mailBoxHardSizelimitValue,
                "hold" => $this->quotaSize);

        $this->groupwareDao->save("quotaSize", $this->uid, $quota);
        // TODO: save Mailbox location
    }


    /*! \brief  Check given values 
     */
    function check()
    {
		//add some checks
        $messages = plugin::check();

        //Check the dates        
        // TODO: check only if features are enabled.
        
        //required vacationEnabled
        if($this->vacationEnabled){
            if(!tests::is_date($this->vacationStart)){
                $messages[] = msgPool::invalid(_("Vacation start date"),$this->vacationStart , "", "01.03.2010");
            }
            if(!tests::is_date($this->vacationStop)){
                $messages[] = msgPool::invalid(_("Vacation stop date"),$this->vacationStop , "", "01.03.2010");
            }
            $diff = tests::compareDate($this->vacationStart, $this->vacationStop);

            if($diff>=0){
                $messages[] = msgPool::invalid(_("Vacation dates"));
            }
        }
        if(!tests::is_email ($this->mailAddress)){
            $messages[] = msgPool::invalid(_("Mail address"),$this->mailAddress , "", "user@exdom.intranet.gonicus.de knibbel");
        }

        if(isset($this->forwardingAddresses) && is_array($this->forwardingAddresses)){
            foreach($this->forwardingAddresses as $fAddress){
                if(!tests::is_email ($fAddress)){
                    $messages[] = msgPool::invalid(_("Alternate address"),$fAddress, "", 
                            "user@exdom.intranet.gonicus.de");
                }
                if($fAddress == $this->mailAddress){
                    $messages[] = _("The primary address cannot be used as alternative address!");
                }
            }
        }
        if(isset($this->alternateAddresses) && is_array($this->alternateAddresses)){
            foreach($this->alternateAddresses as $fAddress){
                if(!tests::is_email ($fAddress)){
                    $messages[] = msgPool::invalid(_("Forward address"),$fAddress, "", "user@excom.intranet.gonicus.de");
                }
                if($fAddress == $this->mailAddress){
                    $messages[] = _("The primary address cannot be used as forward address!");
                }
            }
        }
        
        // TODO: Checks for quota and Locations?
        return($messages);
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
                        "mailFolder"                    => _("Mail folder"),
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


    /*! \brief  Maps the resultset fetched from the Dao to the class variables 
     * 		     of the plugin.
     */
    function mapComprehensiveUserData($callBackMap)
    {

        $map = array(
                "mailLocations" => "mailLocations",
                "mailFolder" => "mailFolder",
                "mailAddress" => "primaryMail",
                "mailLocation" => "mailLocation",
                "quotaUsage" => "quotaUsage",
                "quotaSize" => "quotaSize",
                "alternateAddresses" => "alternateAddresses",
                "forwardingAddresses" => "forwardingAddresses",
                "vacationEnabled" => "vacationEnabled",
                "vacationStart" => "vacationStart",
                "vacationStop" => "vacationStop",
                "vacationMessage" => "vacationMessage",
                "mailBoxWarnLimitEnabled" => "mailBoxWarnLimitEnabled",
                "mailBoxWarnLimitValue" => "mailBoxWarnLimitValue",
                "mailBoxSendSizelimitEnabled" => "mailBoxSendSizelimitEnabled",
                "mailBoxSendSizelimitValue" => "mailBoxSendSizelimitValue",
                "mailBoxHardSizelimitEnabled" => "mailBoxHardSizelimitEnabled",
                "mailBoxHardSizelimitValue" => "mailBoxHardSizelimitValue",
                "mailBoxAutomaticRemovalEnabled" => "mailBoxAutomaticRemovalEnabled",
                "mailBoxAutomaticRemovalValue" => "mailBoxAutomaticRemovalValue",
                "localDeliveryOnly" => "localDeliveryOnly",
                "dropOwnMails" => "dropOwnMails");

        // Map values from source array to class members
        foreach($map as $target => $source){
            if(isset($callBackMap[$source])){
                $this->$target = $callBackMap[$source];
            }
        }
    }
}


?>
