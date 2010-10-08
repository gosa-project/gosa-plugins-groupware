<?php

class FolderEditDialog extends plugin
{
    private $folderItem = NULL;    

    function __construct($config, $folderItem)
    {
        $this->config = $config;
        $this->folderItem = $folderItem;
    }

    function execute()
    {
        $smarty = get_smarty(); 
        $smarty->assign('folderItem', $this->folderItem);
        return($smarty->fetch(get_template_path("FolderEditDialog.tpl", TRUE, dirname(__FILE__))));
    }


    function save_object()
    {
        // Get all posted value modifications
        foreach($this->folderItem['acls'] as $id => $aclEntry){

            // Name or ACL modified?
            if(isset($_POST["permission_{$id}_name"])) {
                $this->folderItem['acls'][$id]['name'] = get_post("permission_{$id}_name");
            }
            if(isset($_POST["permission_{$id}_acl"])) {
                $this->folderItem['acls'][$id]['acl'] = get_post("permission_{$id}_acl");
            }

            // Check if entry has to be removed.
            if(isset($_POST["permission_{$id}_del"])){
                unset($this->folderItem['acls'][$id]);
            }
        }

        // New ACL entry to add?
        if(isset($_POST['permission_add'])){
            $this->folderItem['acls'][] = array('name' => '', 'acl' => 0, 'type' => 'user');
        }
        $this->folderItem['acls'] = array_values($this->folderItem['acls']);
    }

    function save()
    {
        return($this->folderItem);
    }
}

?>
