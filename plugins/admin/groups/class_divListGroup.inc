<?php

class divListGroup extends MultiSelectWindow
{

  /* Current base */
  var $selectedBase       = "";
  var $departments        = array();

  /* Regex */
  var $Regex              = "*";
  var $UserRegex          = "*";

  /* CheckBoxes, to change default values modify $this->AddCheckBox */
  var $ShowPrimaryGroups;
  var $ShowSambaGroups;  
  var $ShowApplicationGroups; 
  var $ShowMailGroups;
  var $ShowFunctionalGroups;

  /* Subsearch checkbox */
  var $SubSearch              = false;

  var $parent             ;
  var $ui                 ;

  function divListGroup ($config,$parent)
  {
    MultiSelectWindow::MultiSelectWindow($config,"Groups");
    
    $this->parent       = $parent;
    $this->ui           = get_userinfo();

    /* Set list strings */
    $this->SetTitle(_("List of groups"));
    $this->SetSummary(_("List of groups"));
    $this->EnableAplhabet   (true);

    /* Result page will look like a headpage */
    $this->SetHeadpageMode();
    $this->SetInformation(_("This menu allows you to add, edit and remove selected groups. You may want to use the range selector on top of the group listbox, when working with a large number of groups."));
  
    /* Disable buttonsm */
    $this->EnableCloseButton(false);
    $this->EnableSaveButton (false);

    /* set Page header */
    $this->AddHeader(array("string"=>"&nbsp;","attach"=>"style='width:20px;'"));
    $this->AddHeader(array("string"=>_("Groupname / Department")));
    $this->AddHeader(array("string"=>_("Properties"),"attach"=>"style='width:136px;'"));
    $this->AddHeader(array("string"=>_("Actions"),"attach"=>"style='width:80px;border-right:0px;'"));

    /*                  Text        ,Value    ,Name         ,Is selected */
    $this->AddCheckBox("ShowPrimaryGroups",     _("Select to see groups that are primary groups of users"),          _("Show primary groups"),      true);
    $this->AddCheckBox("ShowSambaGroups",       _("Select to see groups that have samba groups mappings"),          _("Show samba groups"),      true);
    $this->AddCheckBox("ShowApplicationGroups", _("Select to see groups that have applications configured"),        _("Show application groups"),true);
    $this->AddCheckBox("ShowMailGroups",        _("Select to see groups that have mail settings"),                  _("Show mail groups"),       true);
    $this->AddCheckBox("ShowFunctionalGroups",  _("Select to see normal groups that have only functional aspects"), _("Show functional groups"), true);
    $this->AddCheckBox(SEPERATOR);

    /* Add SubSearch checkbox */    
    $this->AddCheckBox("SubSearch",  _("Select to search within subtrees"), _("Search in subtrees"), false);

    /*                  Name                 ,Text                              ,Default  , Connect with alphabet  */
    $this->AddRegex   ("Regex",     _("Regular expression for matching group names"), "*" , true);
    $this->AddRegex   ("UserRegex", _("User name of which groups are shown"),         "*" , false, "images/search_user.png");
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

    if($this->parent->CopyPasteHandler){
      $Copy_Paste = $this->parent->CopyPasteHandler->generatePasteIcon();
    }else{
      $Copy_Paste ="";
    }

    /* Add default header */
    $listhead = MultiSelectWindow::get_default_header();
    $listhead.= " <input type='image' class='center' src='images/list_new_group.png' 
          title='"._("Create new group")."' alt='"._("New")."' name='group_new'>&nbsp;".
      $Copy_Paste.
      " <img class='center' src='images/list_seperator.png' alt='' height='16' width='1'>&nbsp;".
      _("Base")."&nbsp;<select name='CurrentMainBase' onChange='mainform.submit()' class='center'>$options</select>".
       " <input type='image' class='center' src='images/list_submit.png' 
        title='"._("Submit department")."' name='submit_department' alt='"._("Submit")."'>&nbsp;".
      "</div>";

    $this->SetListHeader($listhead);
  }

  function execute()
  {
    $this->ClearElementsList();
    $this->GenHeader();
  }

  function setEntries($groups)
  {
    // Defining Links
    $linkopen = "<a href='?plug=".$_GET['plug']."&amp;act=dep_open&amp;dep_id=%s'>%s</a>";

    // image Buttons
    $editlink = "<a href='?plug=".$_GET['plug']."&amp;id=%s&amp;act=edit_entry'>%s</a>";
    $userimg  = "<img class='center' src='images/select_groups.png' alt='User'    title='%s'>";

    /* Assign extension images */
    $posiximg = "<input type='image' class='center' src='images/select_groups.png'
                 name='group_group_edit_%KEY%' alt='P'  title='"._("Posix")  ."'>";
    $mailimg  = "<input type='image' class='center' src='images/mailto.png'
                 name='mailgroup_group_edit_%KEY%' alt='M'  title='"._("Mail")   ."'>";
    $sambaimg = "<input type='image'  class='center' src='images/select_winstation.png'
                 name='group_group_edit_%KEY%' alt='S'  title='"._("Samba")  ."'>";
    $applimg  = "<input type='image'  class='center' src='images/select_application.png'
                 name='appgroup_group_edit_%KEY%' alt='A'  title='"._("Application")."'>";
    $phoneimg = "<input type='image'  class='center' src='images/select_phone.png'
                 name='group_group_edit_%KEY%' alt='Ph' title='"._("Phone")  ."'>";
    $envimg   = "<input type='image'  class='center' src='images/smallenv.png'
                 name='environment_group_edit_%KEY%' alt='E' title='"._("Environment")  ."'>";

    // Space
    $empty    = "<img class='center' src='images/empty.png' style='width:16px;height:16px;' alt=''>";


    // User and Template  Images
    $editlink = "<a href='?plug=".$_GET['plug']."&amp;id=%s&amp;act=edit_entry'>%s</a>";

    // Test Every Entry and generate divlist Array
    foreach($groups as $key => $val){

      $acl= get_permissions ($val['dn'], $this->ui->subtreeACL);
      $acl= get_module_permission($acl, "group", $val['dn']);
      


      /* Create action icons - copy & paste icons */
      $actions = "";
      if($this->parent->CopyPasteHandler && $acl == "#all#"){
        $actions.= "<input class='center' type='image'
          src='images/editcut.png' alt='"._("cut")."' name='cut_%KEY%' title='"._("Cut this entry")."'>&nbsp;";
        $actions.= "<input class='center' type='image'
          src='images/editcopy.png' alt='"._("copy")."' name='copy_%KEY%' title='"._("Copy this entry")."'>&nbsp;";
      }
      $actions.= "<input class='center' type='image'
        src='images/edit.png' alt='"._("edit")."' name='group_edit_%KEY%' title='"._("Edit this entry")."'>";

      if(chkacl($acl,"delete") == ""){
      $actions.= "<input class='center' type='image'
        src='images/edittrash.png' alt='"._("delete")."' name='group_del_%KEY%' title='"._("Delete this entry")."'>";
      }




      $posix=$mail=$samba=$appl=$phone=$enviro=$empty;

      if(isset($val['objectClass'])){
        if(in_array("posixGroup",           $val['objectClass']))   $posix = $posiximg;
        if(in_array("gosaMailAccount",      $val['objectClass']))   $mail  = $mailimg;
        if(in_array("sambaGroupMapping",    $val['objectClass']))   $samba = $sambaimg;
        if(in_array("gosaApplicationGroup", $val['objectClass']))   $appl  = $applimg;
        if(in_array("goFonPickupGroup",     $val['objectClass']))   $phone = $phoneimg;
        if(in_array("gotoEnvironment",      $val['objectClass']))   $enviro= $envimg;
      }

      $title = "title='".preg_replace('/ /', '&nbsp;', @LDAP::fix($val['dn']))."'";

      if(!isset($val['description'][0])){
        $desc = "";
      }else{
        $desc = " - [ ".$val['description'][0]." ]";
      }

      /* FAIrelease tag from groupApplications */
      if(isset($val['FAIrelease'][0])){
        $desc .= " (".$val['FAIrelease'][0].")";
      }

      /* Cutted objects should be displayed in light grey */
      $display = $val['cn']['0'].$desc;
      if(isset($this->parent->CopyPasteHandler) && $this->parent->CopyPasteHandler->cutCurrent && $val['dn'] == $this->parent->CopyPasteHandler->objectdn){
        $display = "<font color='#999999'>".$display."</font>";
      }

      $field1 = array("string" => sprintf($userimg,$val['dn']), "attach" => "style='text-align:center;width:20px;'");
      $field2 = array("string" => sprintf($editlink,$key,$display), "attach" => "style='' ".$title);
      $field3 = array("string" => preg_replace("/%KEY%/", $key, $posix."&nbsp;".$enviro."&nbsp;".$mail."&nbsp;".$samba."&nbsp;".$appl."&nbsp;".$phone),     "attach" => "style='width:136px;'");
      $field4 = array("string" => preg_replace("/%KEY%/", $key, $actions), "attach" => "style='width:80px;border-right:0px;text-align:right;'");

      $this->AddElement(array($field1,$field2,$field3,$field4));
    }

    /* Create summary string for list footer */
    $num_deps=0;
    if(!$this->SubSearch){
      $num_deps = count($this->Added_Departments);
    }
    $num_grps = count($groups);

    $num_grp_str = _("Number of listed groups");
    $num_dep_str = _("Number of listed departments");

    $str = "<img class='center' src='images/select_groups.png'
              title='".$num_grp_str."' alt='".$num_grp_str."'>&nbsp;".$num_grps."&nbsp;&nbsp;&nbsp;&nbsp;";
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
    MultiSelectWindow::save_object();   
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>