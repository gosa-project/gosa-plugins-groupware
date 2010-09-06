<?php

class filterEditor extends plugin
{
    function __construct($config, $parent, $filter)
    {
        $this->config = &$config;
        $this->parent = &$parent;
        $this->filter = $filter;

        $this->types = array('OR' => _("or"), 'AND' => _("and"));
        $this->actions = array(
                "MOVE" => _("move mail to"),
                "COPY" => _("copy mail to"),
                "FORWARD" => _("forward message to"),
                "MARK" => _("mark mail as "),
                "DROP" => _("remove mail"),
                "REPLY" => _("reply"));

        $this->fields = array(
                'subject' => _("Subject"),
                'from'=> _("From"),
                'body'=> _("Body"),
                'date'=> _("Date"),
                'priority'=> _("Priority"),
                'status'=> _("Status"),
                'to'=> _("To"),
                'cc'=> _("CC"),
                'age'=> _("Age"),
                'size'=> _("Size"));

        $this->comparators = array(
                "is" => _("is"),
                "is not" => _("is not"), 
                "equal" => _("is equal"),       
                "not equal" => _("is not equal"),   
                "is empty" => _("is empty"),
                "is not empty" => _("is not empty"),
                "contains" => _("contains"),
                "contains not" => _("does not contain"),
                "is in addressbook" => _("is in the addressbook"),
                "is not in addressbook" => _("is not in the addressbook"),
                "greater than" => _("is greater than"),
                "less than" => _("smaller than"));
    }

    function execute()
    {
        $smarty = get_smarty();
        $smarty->assign('NAME',set_post($this->filter['NAME']));    
        $smarty->assign('DESC',set_post($this->filter['DESC']));    
        $smarty->assign('filterStr', $this->renderFilter());    

        return($smarty->fetch(get_template_path('filterEditor.tpl',TRUE,dirname(__FILE__))));
    }

    function renderFilter()
    {
        $filter = $this->filter;

        $cnt = count($filter['CONDITIONS']);
        $str = "<h3>"._("Filter conditions")."</h3>";
        $str .= _("Condition type")."&nbsp;\n<select name='cType' onChange='document.mainform.submit();'>";
        foreach($this->types as $type => $desc){
            $checked = ($type == $filter['TYPE'])? ' selected ' : '';
            $str.= "\n<option {$checked} value=\"".set_post($type)."\">".set_post($desc)."</option>";
        }
        $str .= "\n</select>";
        $str.= "<ul>";
        foreach($filter['CONDITIONS'] as $id => $cond){
            $str .= "<li>";
            $str .= _("Check field")."&nbsp;";
            $str .= "\n<select name='cField_{$id}' onChange='document.mainform.submit();'>";
            foreach($this->fields as $field => $desc){
                $checked = ($field == $cond['FIELD'])? ' selected ' : '';
                $str.= "\n<option {$checked} value=\"".set_post($field)."\">".set_post($desc)."</option>";
            }
            $str .= "\n</select>";
            $str .= "&nbsp;"._("if it")."&nbsp;";
            $str .= "\n<select name='cComparator_{$id}' onChange='document.mainform.submit();'>";
            foreach($this->comparators as $comparator => $desc){
                $checked = ($comparator == $cond['COMPARATOR'])? ' selected ' : '';
                $str.= "\n<option {$checked} value=\"".set_post($comparator)."\">".set_post($desc)."</option>";
            }
            $str .= "\n</select>";
            if(!in_array($cond['COMPARATOR'], array('is in addressbook','is not in addressbook','is empty','is not empty'))){
                $str .= "<input type='text' name='cMatch_{$id}' value=\"".set_post($cond['MATCH'])."\">";
            }
            $cnt --;
            $str .= "</li>";
            if($cnt) $str .= $this->types[$filter['TYPE']];
        } 
        $str .= "</ul>";
        $str .= "<button name='addCondition'>".msgPool::addButton()."</button> ";
        if(count($filter['CONDITIONS']) >= 2){
            $str .= "<button name='removeCondition'>".msgPool::delButton()."</button> ";
        }
       
        $str .= "<hr>";
        $str .= "<h3>"._("Filter actions")."</h3>";

        $str.= "<ul>";
        foreach($filter['ACTION'] as $id => $action){
            $str .= "<li>";
            $str .= "\n<select name='cAction_{$id}' onChange='document.mainform.submit();'>";
            foreach($this->actions as $act => $desc){
                $checked = ($act == $action['ACTION'])? ' selected ' : '';
                $str.= "\n<option {$checked} value=\"".set_post($act)."\">".set_post($desc)."</option>";
            }
            $str .= "</select>";
            if(!in_array($action['ACTION'], array('DROP'))){
                $str .= "<input type='text' name='cValue_{$id}' value=\"".set_post($action['VALUE'])."\">";
            }
            $str .= "</li>";
        }
        $str.= "</ul>";
    
        $str .= "<button name='addAction'>".msgPool::addButton()."</button> ";
        if(count($filter['ACTION']) >= 2){
            $str .= "<button name='removeAction'>".msgPool::delButton()."</button> ";
        }

        return($str);
    }


    function save_object()
    {

        $this->filter['TYPE'] = (isset($_POST['cType']))? get_post('cType'): $this->filter['TYPE'];

        foreach($this->filter['CONDITIONS'] as $id => $value){
            foreach(array(
                        'cField_' => 'FIELD',
                        'cComparator_'=>'COMPARATOR',
                        'cMatch_'=>'MATCH') as $post => $name){
                if(isset($_POST[$post.$id])){
                    $this->filter['CONDITIONS'][$id][$name] = get_post($post.$id);
                }
            }
        }
        foreach($this->filter['ACTION'] as $id => $value){
            if(isset($_POST['cAction_'.$id])){
                $this->filter['ACTION'][$id]['ACTION'] = get_post('cAction_'.$id);
            }
            if(isset($_POST['cValue_'.$id])){
                $this->filter['ACTION'][$id]['VALUE'] = get_post('cValue_'.$id);
            }
        }

        // Add / remove conditions
        if(isset($_POST['removeCondition']) && count($this->filter['CONDITIONS']) >= 2){
            array_pop($this->filter['CONDITIONS']);
        }
        if(isset($_POST['addCondition'])){
            $this->filter['CONDITIONS'][] = array('FIELD' => key($this->fields),'COMPARATOR' => key($this->comparators), 'MATCH' => '');
        }
    
        // Add / remove actions
        if(isset($_POST['removeAction']) && count($this->filter['ACTION']) >= 2){
            array_pop($this->filter['ACTION']);
        }
        if(isset($_POST['addAction'])){
            $this->filter['ACTION'][] = array('ACTION' => 'MOVE', 'VALUE' => '');
        }
    }
}
?>
