<?php

class FolderEditDialog extends plugin
{
    private $item = NULL;    

    function __construct($config, $item)
    {
        $this->config = $config;
        $this->item = $item;
    }

    function execute()
    {
        $smarty = get_smarty(); 
        $smarty->assign('item', $this->item);
        return($smarty->fetch(get_template_path("FolderEditDialog.tpl", TRUE, dirname(__FILE__))));
    }
}


?>
