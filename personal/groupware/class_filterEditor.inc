<?php

class filterEditor extends plugin{

    var $filterListing   = NULL;
    var $filterRules = array();

    function __construct($config, $rules)
    {
        plugin::plugin($config);
        $this->filterRules = $rules;

        $this->filterListing= new sortableListing();
        $this->filterListing->setDeleteable(true);
        $this->filterListing->setEditable(true);
        $this->filterListing->setColspecs(array('*'));
        $this->filterListing->setWidth("100%");
        $this->filterListing->setHeight("200px;");
    }

    function execute()
    {
        $smarty = get_smarty();



        $smarty->assign('list', $this->filterListing->render());

        return($smarty->fetch(get_template_path('filterEditor.tpl',TRUE,dirname(__FILE__))));
    }

}

?>
