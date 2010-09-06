<?php

class filterEditor extends plugin
{
    function __construct($config, $parent, $filter)
    {
        $this->config = &$config;
        $this->parent = &$parent;
        $this->filter = $filter;
    }

    function execute()
    {
        $smarty = get_smarty();

        $smarty->assign('filterStr', $this->renderFilter());    

        return($smarty->fetch(get_template_path('filterEditor.tpl',TRUE,dirname(__FILE__))));
    }

    function renderFilter()
    {
        $str = "";


        return($str);
    }

}
?>
