<?php

class faiProfileEntry extends plugin
{
  /* attribute list for save action */
  var $ignore_account= TRUE;
  var $attributes   = array();
  var $objectclasses= array();

  /* Array with available class names */
  var $FAIAllclasses        = array();

  function faiProfileEntry ($config, $dn= NULL,$used=array(),$objects=false)
  {
    plugin::plugin ($config, $dn);

    /* Search only in fai tree */
    $ObjectTypes = array(
        "FAIpartitionTable"  => array("OU"=>"ou=disk,"       ),
        "FAIpackageList"     => array("OU"=>"ou=packages,"   ),
        "FAIscript"          => array("OU"=>"ou=scripts,"    ),
        "FAIvariable"        => array("OU"=>"ou=variables,"  ),
        "FAIhook"            => array("OU"=>"ou=hooks,"      ),
        "FAIprofile"         => array("OU"=>"ou=profiles,"   ),
        "FAItemplate"        => array("OU"=>"ou=templates,"  ));

    $base= "ou=fai,ou=configs,ou=systems,".$_SESSION['CurrentMainBase'];
    if($_SESSION['faifilter']['branch']!="main"){
      $base = $_SESSION['faifilter']['branch'];
    }

    /* Get ldap connection */
    $ldap= $this->config->get_ldap_link();
    $ldap->cd($base);

    /* Capture objects from given base */
    $result = array();
    foreach($ObjectTypes as $oc => $data){
      $ou = $data['OU'].$base;

      $ldap->ls("(objectClass=".$oc.")",$ou,array("cn","objectClass","dn"));
      while($res = $ldap->fetch()){
        $result[] = $res;
      } 
    }

    /* Create array with categories to be able to sort different types of objects */
    $categories = array("FAIscript","FAItemplate","FAIhook","FAIvariable","FAIpartitionTable","FAIpackageList");
    $sort = array();
    foreach($result as $attrs){   
   
      /* Only use objects which have cn set */
      if((isset($attrs['cn'][0]))&&(!in_array($attrs['cn'][0],$used))){
        foreach($categories as $cat){
          if(in_array($cat,$attrs['objectClass'])){
        
            /* Append different types of objects */
            $this->FAIAllclasses[$attrs['cn'][0]]['objects'][$cat]=$cat;

            /* Set selected status to false */
            $this->FAIAllclasses[$attrs['cn'][0]]['status']=false;

            $sort[strtolower($attrs['cn'][0])] = $attrs['cn'][0];
          }
        }
      }
    }
    $tmp = array();
    ksort($sort);
    foreach($sort as $name){
      $tmp[$name] =$this->FAIAllclasses[$name];
    }
    $this->FAIAllclasses = array();
    $this->FAIAllclasses = $tmp;


    /* If no search filter is set, create one */
    if (!is_global("SUBfaifilter")){
      $SUBfaifilter= array("base" => "ou=fai,ou=configs,ou=systems,".$_SESSION['CurrentMainBase'], "Sregex" => "*");
      $SUBfaifilter['SShowTemplates'] = true;
      $SUBfaifilter['SShowScripts']   = true;
      $SUBfaifilter['SShowHooks']     = true;
      $SUBfaifilter['SShowVariables'] = true;
      $SUBfaifilter['SShowPackages']  = true;
      $SUBfaifilter['SShowPartitions']= true;
      register_global("SUBfaifilter", $SUBfaifilter);
    }
  }

  function execute()
  {
	/* Call parent execute */
	plugin::execute();

    /* Fill templating stuff */
    $smarty     = get_smarty();
    $display = "";

    $SUBfaifilter = $_SESSION['SUBfaifilter'];

    /* If someone used the apply button, update searchfilter */
    if(isset($_POST['Sregex'])){
      foreach($SUBfaifilter as $key => $val){
        if(!isset($_POST[$key])){
          $SUBfaifilter[$key] = false;
        }else{
          $SUBfaifilter[$key] =$_POST[$key];
        }
      }
      $SUBfaifilter['Sregex']= $_POST['Sregex'];
      if(!strstr($SUBfaifilter['Sregex'],"*")){
        $SUBfaifilter['Sregex'] = $SUBfaifilter['Sregex']."*";
      }
    }
  
    /* Save current searchfilter in session*/
    $_SESSION['SUBfaifilter']= $SUBfaifilter;

    /* Check if we used a checkboxe from the list, to select dselect an entry */
    foreach($_POST as $name => $value){
      foreach($this->FAIAllclasses as $class => $obj){
        $bclass = base64_encode($class);
        if(isset($_POST["ON_PAGE_".$bclass])){
          if(isset($_POST['USE_'.$bclass])){
            $this->FAIAllclasses[$class]['status']=true;  
          }else{
            $this->FAIAllclasses[$class]['status']=false;  
          }    
        }
      }
    }

    /* Create different picture for each object type */
    $objTypes['FAIhook']            = "<img src='images/fai_hook.png' title='"._("Hook bundle")."' alt=''>&nbsp;";
    $objTypes['FAItemplate']        = "<img src='images/fai_template.png' title='"._("Template bundle")."' alt=''>&nbsp;";
    $objTypes['FAIscript']          = "<img src='images/fai_script.png' title='"._("Script bundle")."' alt=''>&nbsp;";
    $objTypes['FAIvariable']        = "<img src='images/fai_variable.png' title='"._("Variable bundle")."' alt=''>&nbsp;";
    $objTypes['FAIpackageList']     = "<img src='images/fai_packages.png' title='"._("Package bundle")."' alt=''>&nbsp;";
    $objTypes['FAIpartitionTable']  = "<img src='images/fai_partitionTable.png' title='"._("Partition table")."' alt=''>&nbsp;";

    /* Magic quotes GPC, escapes every ' " \, to solve some security risks
     * If we post the escaped strings they will be escaped again
     */
    foreach($this->attributes as $attrs){
      if(get_magic_quotes_gpc()){
        $smarty->assign($attrs,stripslashes($this->$attrs));
      }else{
        $smarty->assign($attrs,($this->$attrs));
      }
    }

    /* Assign all filter attributes to smarty */
    foreach($SUBfaifilter as $key => $val){
      if(($key != "base")&&($key != "Sregex")){
        if($val){
          $smarty->assign($key."CHK", " checked ");
        }else{
          $smarty->assign($key."CHK", "");
        }
      }else{
        $smarty->assign($key,$val);
      }
    }
  
    /* Create a divlist to display all available class names with the depending object types */
    $divlist = new divlist("ProfileEntry");
    $divlist->SetPluginMode();
    $divlist->SetSummary(_("This list displays all assigned class names for this profile."));
    $divlist->SetEntriesPerPage(0);
    $divlist->SetHeader(array(array("string"=> _("Class name")),
                              array("string"=> _("Objects"),"attach"=>"style='border-right:0px;'")));

    /* Action button allows us to delete an entry */
    $action = "<input type='hidden' name='ON_PAGE_%KEY%' value='1'>
              <input type='checkbox' name='USE_%KEY%' value='%KEY%' onClick='document.mainform.submit();' %CHECK%>";

    /* Check every single class name to match the current search filter */
    foreach($this->FAIAllclasses as $usedClass => $classes){

      $fi = str_replace("*",".*",$SUBfaifilter['Sregex']);  
 
      $abort = true; 
      if(!preg_match("/".$fi."/",$usedClass)){
        $abort = false;
      }
      if((isset($classes['objects']['FAIscript']))&&($SUBfaifilter['SShowScripts'])){
        $abort = false;
      }
      if((isset($classes['objects']['FAItemplate']))&&($SUBfaifilter['SShowTemplates'])){
        $abort = false;
      }
      if((isset($classes['objects']['FAIhook']))&&($SUBfaifilter['SShowHooks'])){
        $abort = false;
      }
      if((isset($classes['objects']['FAIvariable']))&&($SUBfaifilter['SShowVariables'])){
        $abort = false;
      }
      if((isset($classes['objects']['FAIpartitionTable']))&&($SUBfaifilter['SShowPartitions'])){
        $abort = false;
      }
      if((isset($classes['objects']['FAIpackageList']))&&($SUBfaifilter['SShowPackages'])){
        $abort = false;
      }

      if(!$abort){
        $str = "";
        foreach($classes['objects'] as $class => $obj){
          $str.= $objTypes[$obj];
        }

        /* Is this entry selected */ 
        if($classes['status']==true){
          $action_check = preg_replace("/%CHECK%/"," checked ",$action);
        }else{
          $action_check = preg_replace("/%CHECK%/","",$action);
        }      
        
        /* Append to list */
        $field1 = array("string"=> str_replace("%KEY%",base64_encode($usedClass),$action_check).$usedClass,"attach"=>"");
        $field2 = array("string"=> $str,"attach"=>"style='border-right:0px;'");
        $divlist->AddEntry(array($field1,$field2));
      }
    }

    $smarty->assign("faihead"       , "");
    $smarty->assign("failist"       , $divlist->DrawList());
    $smarty->assign("infoimage"     , get_template_path('images/info_small.png'));
    $smarty->assign("launchimage"   , get_template_path('images/small_filter.png'));
    $smarty->assign("alphabet"      , generate_alphabet());
    $smarty->assign("apply"         , apply_filter(TRUE));
    $smarty->assign("search_image"  , get_template_path('images/search.png'));

    $display.= $smarty->fetch(get_template_path('faiProfileEntry.tpl', TRUE));
    return($display);
  }

  /* Save data to object */
  function save_object()
  {
  }

  /* Check supplied data */
  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();

    return ($message);
  }

  function save()
  {
    $tmp = array();
    foreach($this->FAIAllclasses as $class => $obj){
      if($obj['status']==true){
        $tmp[$class]=$class;
      }
    }
    return($tmp);
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>