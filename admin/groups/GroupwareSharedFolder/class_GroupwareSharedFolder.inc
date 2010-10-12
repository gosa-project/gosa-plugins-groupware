<?php


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



/*! \brief  Allows to update groupware shared-folders 
 *           using the jsonRPC backend.
 */
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
    private $FolderWidget = NULL;


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

        $capabilities = array();


        #FIXME
        /* Not working at all, have to wait for backend funtions.
         */
    
#       // Detect supported capabilities 
#       $rpc = $this->config->getRpcHandle();
#       $capabilities = $rpc->gwGetCapabilities();
#       if(!$rpc->success()){
#           $this->rpcError = TRUE;
#           $this->rpcErrorMessage = $rpc->get_error();
#           $message = sprintf(_("Failed to load supported capabilities from server! Error was: '%s'."), 
#                   $rpc->get_error());
#           msg_dialog::display(_("Error"),msgPool::rpcError($message), ERROR_DIALOG);
#           return;
#       }

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
        $is_account = FALSE;
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


    // Returns a dummy folder list . #Fixme remove this.
    function getFolderList()
    {
        $folders = array(
                "shared/Favorites",
                "shared/All Public Folders",
                "shared/All Public Folders/Internet Newsgroups",
                "shared/All Public Folders/testordner",
                "shared/All Public Folders/testordner/klaus ist auch ein ordner",
                "shared/All Public Folders/testordner/klaus ist auch ein ordner/testordner",
                "shared/All Public Folders/testordner/klaus ist auch ein ordner/testordner/xcbdxfgh");

        $ret = array();
        foreach($folders as $folder){
            $name = preg_replace("/^.*\//","",$folder);
            $ret[$folder] = array('status' => '', 'name' => $name, 'acls' => array());
        } 
        return($ret);
    }

    function execute()
    {
        plugin::execute();

        $this->folderList = $this->getFolderList();

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
          Folder editor
         ****************/

        if(isset($_POST['FolderWidget_cancel'])) $this->FolderWidget = NULL;
        if(isset($_POST['FolderWidget_ok'])){
            $this->FolderWidget->save_object();
            $msgs = $this->FolderWidget->check();
            if(count($msgs)){
                msg_dialog::displayChecks($msgs);
            }else{
                $this->folderList = $this->FolderWidget->save();
                $this->FolderWidget = NULL;
            }
        }
        if(isset($_POST['configureFolder'])){
            $this->FolderWidget = new FolderWidget($this->config,$this->folderList, "shared/");
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
          Generate HTML output
         ****************/

        $plInfo = $this->plInfo();

        $smarty = get_smarty();

        foreach($plInfo['plProvidedAcls'] as $name => $translation){
            $smarty->assign("{$name}ACL", $this->getacl($name));
        }

        $smarty->assign('rpcError' , $this->rpcError);
        $smarty->assign('rpcErrorMessage' , $this->rpcErrorMessage);
        return($display.$smarty->fetch(get_template_path('GroupwareSharedFolder/generic.tpl', TRUE)));
    }


    /*! \brief  Get posted values and check which are interesting for us.
     */ 
    function save_object()
    {
        if(isset($_POST['retryInit'])){
            $this->init();
        }
        if(isset($_POST['GroupwareSharedFolder_posted'])){
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


    // Should come from the sever later.  #FIXME
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
}
?>
