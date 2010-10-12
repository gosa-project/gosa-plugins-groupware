<?php

class GroupwareSharedFolder extends plugin
{
    public $view_logged = FALSE;

    // Error hanlding related attributes.
    private $initialized  = FALSE;
    private $rpcError = FALSE;
    private $rpcErrorMessage = "";


    // Attribute definition 
    public $attributes = array('folderList');
    public $folderList = array();   

    // Feature handling
    private $featuresEnabled = array();

    /*! \brief  Constructs the plugin, loads required parent values 
     *           and initiates the initialization.
     */
    function __construct($config, $dn, $attrs = NULL)
    {
        plugin::plugin($config, $dn, $attrs);

        // Get attributes from parent object
        $this->cn = "";
        if(isset($this->attrs['cn'])){
            $this->cn = $this->attrs['cn'][0];
        }
        $this->orig_cn = $this->cn;

        // Initialize the distribution list using the gosa-ng backend 
        $this->init();
    }


    /*! \brief      Check whether a feature is enabled or not.
     *  @param      The feature name to check for
     *  @return     TRUE on success else FALSE
     */
    function featureEnabled($name)
    {
        return(isset($this->featuresEnabled[$name]) && $this->featuresEnabled[$name]);
    }


    /*! \brief      Try to initialize the plugin using a json-rpc connection
     *               to the gosa-ng server.
     */
    function init()
    {
        // Detect supported capabilities 
        $rpc = $this->config->getRpcHandle();
        $capabilities = $rpc->gwGetCapabilities();
        if(!$rpc->success()){
            $this->rpcError = TRUE;
            $this->rpcErrorMessage = $rpc->get_error();
            $message = sprintf(_("Failed to load supported capabilities from server! Error was: '%s'."), 
                    $rpc->get_error());
            msg_dialog::display(_("Error"),msgPool::rpcError($message), ERROR_DIALOG);
            return;
        }

        // Detect features we can use
        $map['folder'] = array("folderList","folderAdd","folderDel","folderExists");
        $map['members'] = array("folderGetMembers","folderSetMembers","folderAddMember","folderDelMember");
        foreach($map as $name => $required){
            $this->featuresEnabled[$name] = TRUE;
            foreach($required as $func){
                $this->featuresEnabled[$name] &= isset($capabilities[$func]) && $capabilities[$func];
            }
        }

        // If we're creating a new ogroup, then we definately have no extension yet.
        $this->rpcError = FALSE;
        $folderList = array();
        if($this->cn == "" || $this->dn == "new"){
            $is_account = FALSE;
        }else{
            
            // Check if account exists
            
        }

        // If account exists then load further info
        if($is_account){
    
            // Load further info
            
        }

        // Store values as current and initial values (saved_attributes) 
        //  to be able to keep track och changes.
        $this->is_account = $this->initially_was_account = $is_account;
        $this->saved_attributes = array();
        $this->folderList = $this->saved_attributes['folderList'] = $folderList;
        $this->capabilities = $capabilities;        
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
            return($smarty->fetch(get_template_path('GroupwareSharedFolder/initFailed.tpl', TRUE)));
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
                $display= $this->show_disable_header(_("Remove shared folder"),
                        msgPool::featuresEnabled(_("Shared folder")));
            } else {
                $display= $this->show_enable_header(_("Create shared folder"),
                        msgPool::featuresDisabled(_("Shared folder")));
                return ($display);
            }
        }


        /****************
          Generate HTML output
         ****************/

        $smarty = get_smarty();
        $smarty->assign('rpcError' , $this->rpcError);
        $smarty->assign('rpcErrorMessage' , $this->rpcErrorMessage);
        return($display.$smarty->fetch(get_template_path('GroupwareSharedFolder/generic.tpl', TRUE)));
    }


    /*! \brief  ACL settings
     */
    static function plInfo()
    {
        return (array(
                    "plShortName"     => _("Shared folder"),
                    "plDescription"   => _("Groupware shared folder"),
                    "plSelfModify"    => FALSE,
                    "plDepends"       => array("ogroup"),                     // This plugin depends on
                    "plPriority"      => 4,                                 // Position in tabs
                    "plSection"     => array("administration"),
                    "plCategory"    => array("groups"),
                    "plOptions"       => array(),
                    "plProvidedAcls"  => array(
                        "folderList"   => _("Shared folder"))
                    ));
    }



    /*! \brief  Get posted values and check which are interesting for us.
     */ 
    function save_object()
    {
        if(isset($_POST['GroupwareSharedFolder_posted'])){
            if(isset($_POST['retryInit'])){
                $this->init();
            }
            plugin::save_object();
        }
    }



    /*!   \brief    Removes the distribution list extension for the current 
     *               object group.
     */  
    function remove_from_parent()
    {
    }


    /*!   \brief    Saves the distribution list extension for the current 
     *               object group.
     */  
    function save()
    {
    }


    function check()
    {
        $messages = plugin::check();

        // Get current object-group name maybe it is invalid for us.
        if(isset($this->parent->by_object['ogroup']->cn)){
            $this->cn = &$this->parent->by_object['ogroup']->cn;
        }
        return($messages);
    }
}
?>
