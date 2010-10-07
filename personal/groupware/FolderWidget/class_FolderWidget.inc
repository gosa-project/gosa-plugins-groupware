<?php




class FolderWidget extends plugin
{
    public $config;
    private $FolderWidgetListing = NULL;

    function __construct(&$config)
    {
        $this->FolderWidgetListing = new FolderWidgetListing($this->config, get_userinfo());
    }


    function execute()
    {
        return($this->FolderWidgetListing->execute());
    }
}

?>
