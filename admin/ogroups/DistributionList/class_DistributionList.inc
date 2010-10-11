<?php

class DistributionList extends plugin
{

    private $initialized  = FALSE;
    private $rpcError = FALSE;
    private $rpcErrorMessage = "";

    public $view_logged = FALSE;

    public $attributes = array('primaryMailAddress','alternateAddresses','memberList');


    function __construct($config, $dn, $attrs = NULL)
    {
        plugin::plugin($config, $dn, $attrs);

        // Get attributes from parent object
        foreach(array("uid","cn") as $attr){
            if(isset($this->parent->by_object['group']) && isset($this->parent->by_object['group']->$attr)){
                $this->$attr = &$this->parent->by_object['group']->$attr;
            }elseif(isset($this->attrs[$attr])){
                $this->$attr = $this->attrs[$attr][0];
            }
        }

        // Initialize the distribution list using the gosa-ng backend 
        $this->init();
    }

    function init()
    {
        // Check whether a mathing distribution-list exsits or not?
        $rpc = $this->config->getRpcHandle();
        $is_account = $rpc->gwDistExists($this->cn);
        $this->rpcError = FALSE;
         
#       // An error occured abort here
#       if(!$rpc->success()){
#           $this->rpcError = TRUE;
#           $this->rpcErrorMessage = $rpc->get_error();
#           msg_dialog::display(_("Error"),msgPool::rpcError($this->rpcErrorMessage), ERROR_DIALOG);
#           return;
#       }
#
#       // We've detected a valid distribution list, now load all
#       //  configured members, so we're able to update the memberlist
#       //  on save();
#       $memberList = array();
#       $primaryMailAddress = "";
#       if($is_account){
#   
#           // Load list of members 
#           $memberList = $rpc->gwDistGetMembers($this->cn);
#           if(!$rpc->success()){
#               $this->rpcError = TRUE;
#               $this->rpcErrorMessage = $rpc->get_error();
#               msg_dialog::display(_("Error"),msgPool::rpcError($this->rpcErrorMessage), ERROR_DIALOG);
#               return;
#           }
#
#           // Now get the primary mail address
#           $primaryMailAddress = $rpc->gwDistGetPrimaryMailAddress($this->cn); 
#           if(!$rpc->success()){
#               $this->rpcError = TRUE;
#               $this->rpcErrorMessage = $rpc->get_error();
#               msg_dialog::display(_("Error"),msgPool::rpcError($this->rpcErrorMessage), ERROR_DIALOG);
#               return;
#           }
#
#           // Load alternate mail address 
#           $alternateAddresses = $rpc->gwDistGetAlternateMailAddresses($this->cn); 
#           if(!$rpc->success()){
#               $this->rpcError = TRUE;
#               $this->rpcErrorMessage = $rpc->get_error();
#               msg_dialog::display(_("Error"),msgPool::rpcError($this->rpcErrorMessage), ERROR_DIALOG);
#               return;
#           }
#           
#       }

        $is_account = TRUE;
        $primaryMailAddress = 'technik@gonicus.de';
        $memberList = 'hape';
        $alternateAddresses = array("schinken@gonicus.de","wurst@gonicus.de");

        // Store values as current and initial values (saved_attributes) 
        //  to be able to keep track och changes.
        $this->is_account = $this->initially_was_account = $is_account;
        $this->saved_attributes = array();
        $this->memberList = $this->saved_attributes['memberList'] = $memberList;
        $this->primaryMailAddress = $this->saved_attributes['primaryMailAddress'] = $primaryMailAddress;
        $this->alternateAddresses = $this->saved_attributes['alternateAddresses'] = $alternateAddresses;
        $this->initialized = TRUE;
    }


    function execute()
    {
        plugin::execute();

        // Initialization failed - Display a stripped template which allows 
        //  to retry initialization
        if(!$this->initialized){
            $smarty = get_smarty();
            $smarty->assign('rpcError' , $this->rpcError);
            $smarty->assign('rpcErrorMessage' , $this->rpcErrorMessage);
            return($smarty->fetch(get_template_path('DistributionList/initFailed.tpl', TRUE)));
        }

        // Log account access
        if($this->is_account && !$this->view_logged){
            $this->view_logged = TRUE;
            new log("view","ogroups/".get_class($this),$this->dn);
        }

        // Allow to add or remove the distribution list extension 
        if(isset($_POST['modify_state'])){
            if($this->is_account && $this->acl_is_removeable()){
                $this->is_account= FALSE;
            }elseif(!$this->is_account && $this->acl_is_createable()){
                $this->is_account= TRUE;
            }
        }

        // Show account status-changer
        if ($this->parent !== NULL){
            if ($this->is_account){
                $display= $this->show_disable_header(_("Remove distribution list"),
                        msgPool::featuresEnabled(_("distribution list")));
            } else {
                $display= $this->show_enable_header(_("Create distribution list"),
                        msgPool::featuresDisabled(_("distribution list")));
                return ($display);
            }
        }

        /****************
          Alternate addresses
         ****************/
        // Add manually inserted alternate mail address.
        if (isset($_POST['addAlternateAddress'])){
            $valid= FALSE;
            if (!tests::is_email($_POST['alternateAddressInput'])){
                msg_dialog::display(_("Error"),msgPool::invalid(_("Mail address"),
                            "","","your-domain@your-domain.com"),ERROR_DIALOG);
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



    



        // Generate HTML output
        $smarty = get_smarty();
        $smarty->assign('rpcError' , $this->rpcError);
        foreach(array("primaryMailAddress","alternateAddresses") as $attr){
            $smarty->assign("{$attr}ACL", $this->getacl($attr));
            $smarty->assign($attr, set_post($this->$attr));
        }

        $smarty->assign("memberList", set_post($this->memberList));
        $smarty->assign('rpcErrorMessage' , $this->rpcErrorMessage);
        return($display.$smarty->fetch(get_template_path('DistributionList/generic.tpl', TRUE)));
    }


    /*! \brief  ACL settings
     */
    static function plInfo()
    {
        return (array(
                    "plShortName"     => _("Distribution list"),
                    "plDescription"   => _("Groupware distribution lists"),
                    "plSelfModify"    => FALSE,
                    "plDepends"       => array("ogroup"),                     // This plugin depends on
                    "plPriority"      => 4,                                 // Position in tabs
                    "plSection"     => array("administration"),
                    "plCategory"    => array("ogroups"),
                    "plOptions"       => array(),
                    "plProvidedAcls"  => array(
                        "primaryMailAddress"   => _("Mail address"),
                        "alternateAddresses"   => _("Alternate mail addresses"))
                        ));
    }


 

    function save_object()
    {
    
        if(isset($_POST['retryInit'])){
            $this->init();
        }
        plugin::save_object();
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
}
?>
