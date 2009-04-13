<?php

class faiScriptEntry extends plugin
{
  /* attribute list for save action */
  var $ignore_account= TRUE;
  var $attributes   = array("cn","description","FAIpriority","FAIscript");
  var $objectclasses= array();

  var $orig_cn              = "";

  var $dn            = "";
  var $cn            = "";
  var $FAIpriority   = "0";
  var $FAIscript     = "";
  var $description   = "";
  var $status        = "new";
  var $parent        = NULL;
  var $FAIstate      = "";
  
  function faiScriptEntry ($config, $dn= NULL,$object=false)
  {
    plugin::plugin ($config, $dn);
    if($dn != "new"){
      $this->orig_cn= $object['cn'];
      $this->dn=$object['dn'];
      foreach($object as $name=>$value){
        $oname = $name;
        $this->$oname=$value;
      }

      if(isset($this->attrs['FAIstate'][0])){
        $this->FAIstate = $this->attrs['FAIstate'][0];
      }

    }else{
      if(is_array($object)&&count($object)){
        $this->orig_cn= $object['cn'];
        $this->dn=$object['dn'];
        foreach($object as $name=>$value){
          $oname = $name;
          $this->$oname=$value;
        }
      }else{

        $this->status = "new";
        $this->orig_cn       = false;
      }
    }
  }

  function execute()
  {
	/* Call parent execute */
	plugin::execute();

    /* Fill templating stuff */
    $smarty     = get_smarty();
    $display = "";
  
    if(isset($_POST['ImportUpload'])){
      if(($_FILES['ImportFile']['error']!=0)){
        print_red(_("Please select a valid file."));
      }else
      if(($_FILES['ImportFile']['size']==0)){
        print_red(_("Selected file is empty."));
      }else{
        $str = utf8_encode(addslashes(file_get_contents($_FILES['ImportFile']['tmp_name'])));
        $this->FAIscript = $str;
      }
    }
  
    foreach($this->attributes as $attrs){
      if(get_magic_quotes_gpc()){
       $smarty->assign($attrs,stripslashes($this->$attrs));
      }else{
       $smarty->assign($attrs,($this->$attrs));
      } 
    }

     /* Create download button*/
    if($this->dn != "new"){
      $smarty->assign("DownMe","<a href='getFAIscript.php?id=".base64_encode($this->dn)."'>
        <input type='button' value='"._("Download")."'>
        </a>");
    }else{
      $smarty->assign("DownMe","");
    }

    for($i =0 ; $i < 100 ; $i++){
      $FAIprioritys[$i]=$i;
    }

    foreach($this->attributes as $attr){
      if(($this->FAIstate == "freeze") || (chkacl($this->acl,$attr)!= "")){
        $smarty->assign($attr."ACL"," disabled ");
      }else{
        $smarty->assign($attr."ACL","  ");
      }
    }

    $smarty->assign("FAIprioritys",$FAIprioritys);
    $display.= $smarty->fetch(get_template_path('faiScriptEntry.tpl', TRUE));
    return($display);
  }

  /* Save data to object */
  function save_object()
  {
    if((isset($_POST['SubObjectFormSubmitted'])) && ($this->FAIstate != "freeze")){
      foreach($this->attributes as $attrs){
        if(isset($_POST[$attrs])){
          $this->$attrs = $_POST[$attrs];
        }else{
          $this->$attrs = "";
        }
      }
    }
  }

  /* Check supplied data */
  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();
  
    if(isset($this->parent->SubObjects[$this->cn]) && $this->cn != $this->orig_cn){
      $message[] =_("There is already a script with the given name.");
    }

    if(empty($this->cn)){
      $message[] = _("Please enter a name.");
    }

    if(empty($this->FAIscript)){
      $message[]=_("Please enter a script.");
    }

    return ($message);
  }
 
  function save()
  {
    $tmp=array();
    foreach($this->attributes as $attrs){ 
      $tmp[$attrs] = $this->$attrs;
    }

    /* Strip out dos newlines */
    $tmp['FAIscript']= strtr($this->FAIscript, array("\x0D" => ""));

    if(($this->orig_cn)&&($tmp['cn']!=$this->orig_cn)){
      $tmp['remove']['from']  = $this->orig_cn;
      $tmp['remove']['to']    = $tmp['cn'];
    }
  
    $tmp['dn']      = $this->dn;  
    $tmp['status']  = $this->status; 
    return($tmp);
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>