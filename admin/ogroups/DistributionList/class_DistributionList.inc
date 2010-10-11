<?php

class DistributionList extends plugin
{

    private $initialized  = FALSE;
    private $rpcError = FALSE;
    private $rpcErrorMessage = "";

    function __construct($config, $dn, $attrs)
    {
        plugin::plugin($config, $dn, $attr);

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
        
        // An error occured abort here
        if(!$rpc->success()){
            $this->rpcError = TRUE;
            $this->rpcErrorMessage = $rpc->get_error();
            msg_dialog::display(msgPool::rpcError($this->rpcErrorMessage));
            return;
        }

        // We've detected a valid distribution list, now load all
        //  configured members, so we're able to update the memberlist
        //  on save();
        $memberList = array();
        $primaryMailAddress = "";
        if($is_account){
    
            // Load list of members 
            $memberList = $rpc->gwDistGetMembers($this->cn);
            if(!$rpc->success()){
                $this->rpcError = TRUE;
                $this->rpcErrorMessage = $rpc->get_error();
                msg_dialog::display(msgPool::rpcError($this->rpcErrorMessage));
                return;
            }

            // Now get the primary mail address
            $primaryMailAddress = $rpc->gwDistGetPrimaryMailAddress($this->cn); 
            if(!$rpc->success()){
                $this->rpcError = TRUE;
                $this->rpcErrorMessage = $rpc->get_error();
                msg_dialog::display(msgPool::rpcError($this->rpcErrorMessage));
                return;
            }

            // Load alternate mail address 
            $alternateAddresses = $rpc->gwDistGetAlternateMailAddresses($this->cn); 
            if(!$rpc->success()){
                $this->rpcError = TRUE;
                $this->rpcErrorMessage = $rpc->get_error();
                msg_dialog::display(msgPool::rpcError($this->rpcErrorMessage));
                return;
            }
            
        }

        $this->is_account = $is_account;
        $this->memberList = $memberList;
        $this->primaryMailAddress = $primaryMailAddress;
        $this->alternateAddresses = $alternateAddresses;
    }


    function execute()
    {
        plugin::execute();

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
        return($display);
    }
}

?>
