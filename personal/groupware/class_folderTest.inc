<?php

class FolderTest extends plugin 
{

    var $FolderWidget = NULL;

    function __construct($config, $dn)
    {
        plugin::plugin($config, $dn);
        $this->folderList = $this->getFolderList();
    }

    
    function execute()
    {

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
            $this->FolderWidget = new FolderWidget($this->config,$this->folderList, "");
            $this->FolderWidget->acl_base = $this->acl_base;
            $this->FolderWidget->acl_category = $this->acl_category;
        }
        $this->dialog = FALSE;
        if($this->FolderWidget instanceOf FolderWidget){
            $this->FolderWidget->save_object();
            $this->dialog = TRUE;
            return($this->FolderWidget->execute());
        }
   
        return("<button name='configureFolder'>edit</button>") ;
    }


    function getFolderList()
    {
        
        $ret['user/wiwu/Inbox/Aufgaben']['name'] = "Aufgaben"; 
        $ret['user/wiwu/Inbox/Aufgaben']['status'] = '';
        $ret['user/wiwu/Inbox/Aufgaben']['type'] = 'user';
        $ret['user/wiwu/Inbox/Aufgaben']['acls'] = array();

        $ret['user/wiwu/Inbox/Gesendet']['name'] = "Gesendet"; 
        $ret['user/wiwu/Inbox/Gesendet']['status'] = '';
        $ret['user/wiwu/Inbox/Gesendet']['type'] = 'user';
        $ret['user/wiwu/Inbox/Gesendet']['acls'] = array();

        $ret['user/wiwu/Inbox/Gesendet/2009']['name'] = "2009"; 
        $ret['user/wiwu/Inbox/Gesendet/2009']['status'] = '';
        $ret['user/wiwu/Inbox/Gesendet/2009']['type'] = 'user';
        $ret['user/wiwu/Inbox/Gesendet/2009']['acls'] = array();

        $ret['user/wiwu/Inbox/Gesendet/2010']['name'] = "2010"; 
        $ret['user/wiwu/Inbox/Gesendet/2010']['status'] = '';
        $ret['user/wiwu/Inbox/Gesendet/2010']['type'] = 'user';
        $ret['user/wiwu/Inbox/Gesendet/2010']['acls'] = array();

        return($ret);
    }
}

?>
