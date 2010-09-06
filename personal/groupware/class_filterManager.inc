<?php

class filterManager extends plugin{

    var $filterListing   = NULL;
    var $filterRules = array();

    function __construct($config,$parent,$rules)
    {
        plugin::plugin($config);
        
        $this->parent = &$parent;

        $this->filterRules = $rules;
        $this->filterListing= new sortableListing();
        $this->filterListing->setDeleteable(true);
        $this->filterListing->setEditable(true);
        $this->filterListing->setColspecs(array('*'));
        $this->filterListing->setWidth("100%");
        $this->filterListing->setHeight("150px;");
        $this->filterListing->setAcl($this->parent->getacl('mailFilter'));


        $this->filter = array(); 
        $this->filter[] =   array (
                'STATUS' => 'EXISTS',
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
                'STATUS' => 'NEW',
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

    }

    function execute()
    {
        // Display filter editor while a filter rule is edited
        if($this->filterEditor instanceOf filterEditor){
            $this->filterEditor->save_object();
            return($this->filterEditor->execute());
        }
        

        $smarty = get_smarty();
        $data = $lData = array();
        foreach($this->filter as $key => $filter){
            $data[$key] = $filter;

            switch($filter['STATUS']){
                case 'NEW' : $img = image('images/lists/element.png[new]');break;
                case 'EXISTS' : $img = image('images/lists/element.png');break;
                default : $img = "";
            }

            $lData[$key] = array('data' => array($img,$filter['NAME'], $filter['DESC']));
        }
        $this->filterListing->setListData($data,$lData);
        $this->filterListing->update();



        $smarty->assign('list', $this->filterListing->render());

        return($smarty->fetch(get_template_path('filterManager.tpl',TRUE,dirname(__FILE__))));
    }

    function save_object()
    {
        $this->filterListing->save_object();
        $action = $this->filterListing->getAction();

        // Remove filter was requested.
        if($action['action'] == 'delete'){
            $key = $action['targets'][0];
            $key = $this->filterListing->getKey($key);
            if(isset($this->filter[$key])){
                unset($this->filter[$key]);
                $this->filter = array_values($this->filter);
            }
        }

        // Edit filter was requested.
        if($action['action'] == 'edit'){
            $key = $action['targets'][0];
            $key = $this->filterListing->getKey($key);
            if(isset($this->filter[$key])){
                $filter = $this->filter[$key];
                $this->filterEditor = new filterEditor($this->config,$this->parent, $filter);
                $this->currentFilter = $key;
            }
        }

        // Close filter editor 
        if(isset($_POST['filterEditor_cancel'])){
            $this->currentFilter = NULL;
            $this->filterEditor = NULL;
        }

        // Save filter modifications and close the dialog
        if(isset($_POST['filterEditor_ok'])){
            $this->filterEditor = NULL;
            $this->currentFilter = NULL;
        }
    }
}

?>
