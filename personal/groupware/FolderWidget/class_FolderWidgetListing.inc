<?php

class FolderWidgetListing extends management
{
    function __construct($config,$ui)
    {
        $this->config = $config;
        $this->ui = $ui;

        $this->storagePoints = array("");

        // Build filter
        if (session::global_is_set(get_class($this)."_filter")){
            $filter= session::global_get(get_class($this)."_filter");
        } else {
            $filter = new filter(get_template_path("FolderWidget-filter.xml", true, dirname(__FILE__)));
            $filter->setObjectStorage($this->storagePoints);
        }
        $this->setFilter($filter);

        // Build headpage
        $headpage = new listing(get_template_path("FolderWidget-list.xml", true, dirname(__FILE__)));
        $headpage->setFilter($filter);

        parent::__construct($config, $ui, "Folder", $headpage);
    }


}


?>
