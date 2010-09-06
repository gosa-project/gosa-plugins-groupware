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

        $this->filter = array(); 
        $this->filter[] =   array (
                'TYPE' => 'AND',
                'NAME' => 'MuellRaus',
                'DESC' => 'Spam loeschen',
                'CONDITIONS' => array  (
                    array('FIELD' => 'from',
                        'COMPARATOR' => 'equals',
                        'MATCH' => 'herbert'),
                    array('FIELD' => 'subject',
                        'COMPARATOR' => 'contains',
                        'MATCH' => 'advertising'),
                    ),
                'ACTION' => array (
                    array('ACTION'=>'MARK',
                        'VALUE' => 'SPAM'),
                    array('ACTION'=>'MOVE',
                        'VALUE' => 'gosa+shared/hickert/trash')
                    )
                );

        $this->filter[] =   array (
                'TYPE' => 'OR',
                'NAME' => 'Kunden',
                'DESC' => 'Kunden Mails in Wichtig schieben',
                'CONDITIONS' => array  (
                    array('FIELD' => 'from',
                        'COMPARATOR' => 'contains',
                        'MATCH' => 'gonicus'),
                    array('FIELD' => 'from',
                        'COMPARATOR' => 'contains',
                        'MATCH' => 'lka'),
                    ),
                'ACTION' => array (
                    array('ACTION'=>'MARK',
                        'VALUE' => 'HAM'),
                    array('ACTION'=>'MOVE',
                        'VALUE' => 'gosa+shared/hickert/wichtig')
                    )
                );


        $smarty = get_smarty();



        $smarty->assign('list', $this->filterListing->render());

        return($smarty->fetch(get_template_path('filterEditor.tpl',TRUE,dirname(__FILE__))));
    }

}

?>
