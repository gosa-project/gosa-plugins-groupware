<?php

class divListMacro extends MultiSelectWindow
{

  /* Current base */
  var $selectedBase       = "";
  var $departments        = array();

  /* Regex */
  var $Regex              = "*";

  /* checkboxes */
  var $ShowSendBocklists      ;
  var $ShowReceiveMacros  ;

  /* Subsearch checkbox */
  var $SubSearch;

  var $parent             ;
  var $ui                 ;

  function divListMacro ($config,$parent)
  {
    MultiSelectWindow::MultiSelectWindow($config,"Macros");
    
    $this->parent       = $parent;
    $this->ui           = get_userinfo();

    /* Set list strings */
    $this->SetTitle(_("List of macros"));
    $this->SetSummary(_("List of macros"));

    /* Result page will look like a headpage */
    $this->SetHeadpageMode();
    $this->SetInformation(_("This menu allows you to add, edit and remove selected macros. You may want to use the range selector on top of the macro listbox, when working with a large number of macros."));

    $this->EnableAplhabet   (true);
  
    /* Disable buttonsm */
    $this->EnableCloseButton(false);
    $this->EnableSaveButton (false);

    /* set Page header */
    $this->AddHeader(array("string" => "&nbsp;", "attach" => "style='text-align:center;width:20px;'"));
    $this->AddHeader(array("string" => _("macro name")." / "._("Department"), "attach" => "style=''"));
    $this->AddHeader(array("string" => _("Visible"), "attach" => "style='width:50px;'"));
    $this->AddHeader(array("string" => _("Actions"), "attach" => "style='width:60px;border-right:0px;text-align:right;'" ));

    /* Add Checkboxes / SubSearch checkbox */
    $this->AddCheckBox("SubSearch",  _("Select to search within subtrees"), _("Search in subtrees"), false);

    /*                  Name                 ,Text                              ,Default  , Connect with alphabet  */
    $this->AddRegex   ("Regex",     _("Regular expression for matching macro names"),"*" , true);
  }

  function GenHeader()
  {
    /* Prepare departments,
       which are shown in the listbox on top of the listbox
     */
    $options= "";
    foreach ($this->config->idepartments as $key => $value){
      if ($this->selectedBase == $key){
        $options.= "<option selected='selected' value='$key'>$value</option>";
      } else {
        $options.= "<option value='$key'>$value</option>";
      }
    }

    /* Add default header */
    $listhead = MultiSelectWindow::get_default_header();
    $listhead.=  " <input class='center' type='image' align='middle' src='images/list_new_macro.png' 
        title='"._("Create new phone macro")."' alt='"._("New")."' name='user_new'>&nbsp;".
      " <img class='center' src='images/list_seperator.png' align='middle' alt='' height='16' width='1'>&nbsp;".
      _("Base")."&nbsp;<select name='CurrentMainBase' onChange='mainform.submit()' class='center'>$options</select>".
      " <input class='center' type='image' src='images/list_submit.png' align='middle' 
        title='"._("Submit department")."' name='submit_department' alt='"._("Submit")."'>&nbsp;".
      "</div>";

    $this->SetListHeader($listhead);
  }

  function execute()
  {
    $this->ClearElementsList();
    $this->GenHeader();
  }

  function setEntries($list)
  {
  
    $action     = "<input class='center' type='image' src='images/edit.png' alt='"._("edit")."'     
                    name='user_edit_%KEY%' title='"._("Edit macro")."'>";
    $action    .= "<input class='center' type='image' src='images/edittrash.png' alt='"._("delete")."'   
                    name='user_del_%KEY%' title='"._("Delete macro")."'>";
  
    $empty      = "<img class='center' src='images/nothing.png' style='width:16px;height:16px;' alt=''>";
    $macroimg   = "<img class='center' src='images/list_macro.png' alt='"._("Macro")."' title='%s'>";
    $visible    = "<img class='center' src='images/true.png' alt='"._("yes")."' title='"._("visible")."'>";
    $invisible  = "<img class='center' src='images/false.png' alt='"._("no")."'title='"._("invisible")."'>";
    $editlink   = "<a href='?plug=".$_GET['plug']."&amp;id=%s&amp;act=edit_entry'>%s</a>";

    foreach($list as $key => $val){
      $display= $val["displayName"][0];

      if(isset($val['description'][0])){

        $desc = $val['description'][0];
        if(strlen($desc) > 55){
          $desc = substr($desc,0,52)."...";
        }

        $display .= "&nbsp;[".$desc."]";
      }

      if(isset($val['goFonMacroVisible']['0'])&&($val['goFonMacroVisible']['0'] == "1")){
        $pic1 = $visible;
      }else{
        $pic1 = $invisible;
      }

      $field1 = array("string" => sprintf($macroimg,$val['dn']), "attach" => "style='text-align:center;width:20px;'");
      $field2 = array("string" => sprintf($editlink,$key,$display), "attach" => "style='' title='".preg_replace('/ /', '&nbsp;', @LDAP::fix($val['dn']))."'");
      $field3 = array("string" => $pic1, "attach" => "style='width:50px;'");
      $field4 = array("string" => preg_replace("/%KEY%/", "$key", $action), "attach" => "style='width:60px;border-right:0px;text-align:right;'");

      $this->AddElement(array($field1,$field2,$field3,$field4));
    }

    /* Create summary string for list footer */
    $num_deps=0;
    if(!$this->SubSearch){
      $num_deps = count($this->Added_Departments);
    }
    $num_objs = count($list);

    $num_obj_str = _("Number of listed macros");
    $num_dep_str = _("Number of listed departments");

    $str = "<img class='center' src='images/select_macro.png'
              title='".$num_obj_str."' alt='".$num_obj_str."'>&nbsp;".$num_objs."&nbsp;&nbsp;&nbsp;&nbsp;";
    $str.= "<img class='center' src='images/folder.png'
              title='".$num_dep_str."' alt='".$num_dep_str."'>&nbsp;".$num_deps."&nbsp;&nbsp;&nbsp;&nbsp;";

    $this->set_List_Bottom_Info($str);
  }

  function Save()
  {
    MultiSelectWindow :: Save();  
  }

  function save_object()
  {
    /* Save automatic created POSTs like regex, checkboxes */
    MultiSelectWindow :: save_object();   
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>