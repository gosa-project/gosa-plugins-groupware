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
        $smarty->assign('NAME',set_post($this->filter['NAME']));    
        $smarty->assign('DESC',set_post($this->filter['DESC']));    
        $smarty->assign('filterStr', $this->renderFilter());    

        return($smarty->fetch(get_template_path('filterEditor.tpl',TRUE,dirname(__FILE__))));
    }

    function renderFilter()
    {
        $filter = $this->filter;

        $actions = array(
                "MOVE" => _("move mail to"),
                "COPY" => _("copy mail to"),
                "FORWARD" => _("forward message to"),
                "MARK" => _("mark mail as "),
                "DROP" => _("remove mail"),
                "REPLY" => _("reply"));

        $fields = array(
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

        $comparators = array(
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

        

        $cnt = count($filter['CONDITIONS']);
        $str = "<h3>"._("Filter conditions")."</h3>";
        $str.= "<ul>";
        foreach($filter['CONDITIONS'] as $id => $cond){
            $str .= "<li>";
            $str .= _("Check field")."&nbsp;";
            $str .= "\n<select name='cField_{$id}' onChange='document.mainform.submit();'>";
            foreach($fields as $field => $desc){
                $checked = ($field == $cond['FIELD'])? ' selected ' : '';
                $str.= "\n<option {$checked} value=\"".set_post($field)."\">".set_post($desc)."</option>";
            }
            $str .= "\n</select>";
            $str .= "&nbsp;"._("if it")."&nbsp;";
            $str .= "\n<select name='cComparator_{$id}' onChange='document.mainform.submit();'>";
            foreach($comparators as $comparator => $desc){
                $checked = ($comparator == $cond['COMPARATOR'])? ' selected ' : '';
                $str.= "\n<option {$checked} value=\"".set_post($comparator)."\">".set_post($desc)."</option>";
            }
            $str .= "\n</select>";
            if(!in_array($cond['COMPARATOR'], array('is in addressbook','is not in addressbook','is empty','is not empty'))){
                $str .= "<input type='text' name='cMatch_{$id}' value=\"".set_post($cond['MATCH'])."\">";
            }
            $cnt --;
            $str .= "</li>";
            if($cnt) $str .= $filter['TYPE'];
        } 
        $str .= "</ul>";
       
        $str .= "<hr>";
        $str .= "<h3>"._("Filter actions")."</h3>";

        $str.= "<ul>";
        foreach($filter['ACTION'] as $id => $action){
            $str .= "<li>";
            $str .= "\n<select name='cAction_{$id}' onChange='document.mainform.submit();'>";
            foreach($actions as $act => $desc){
                $checked = ($act == $action['ACTION'])? ' selected ' : '';
                $str.= "\n<option {$checked} value=\"".set_post($act)."\">".set_post($desc)."</option>";
            }
            $str .= "</select>";
            if(!in_array($action['ACTION'], array('remove'))){
                $str .= "<input type='text' name='cValue_{$id}' value=\"".set_post($action['VALUE'])."\">";
            }
            $str .= "</li>";
        }
        $str.= "</ul>";
    

        return($str);
    }


    function save_object()
    {
        foreach($this->filter['CONDITIONS'] as $id => $value){
            if(isset($_POST['cField_'.$id])){
                $this->filter['CONDITIONS'][$id]['FIELD'] = get_post('cField_'.$id);
            }
            if(isset($_POST['cComparator_'.$id])){
                $this->filter['CONDITIONS'][$id]['COMPARATOR'] = get_post('cComparator_'.$id);
            }
            if(isset($_POST['cMatch_'.$id])){
                $this->filter['CONDITIONS'][$id]['MATCH'] = get_post('cMatch_'.$id);
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
    }
}
?>
