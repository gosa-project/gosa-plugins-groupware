<?php

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


class FolderTest extends plugin 
{
    var $FolderWidget = NULL;

    function __construct($config, $dn)
    {
        plugin::plugin($config, $dn);
        $this->folderList = $this->getFolderList();
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
            $this->FolderWidget = new FolderWidget($this->config,$this->folderList, "user/wiwu/Inbox");
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
