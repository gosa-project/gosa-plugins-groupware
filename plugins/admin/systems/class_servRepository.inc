<?php

class servrepository extends plugin
{
  /* attribute list for save action */
  //  var $ignore_account         = TRUE;
  var $attributes             = array("FAIrepository");
  var $objectclasses          = array("FAIrepositoryServer");
  
  /* Search filter */
  var $regex                  = "*";

  /* Configurationdialog for repositories */
  var $dialog                 = NULL;

  /* Repositories */
  var $repositories           = array();
  var $FAIrepository          = array();

  var $fai_activated          = FALSE;

  function servrepository ($config, $dn= NULL, $parent= NULL)
  {
    plugin::plugin ($config, $dn, $parent);

    /* Skip this if fai is deactivated */
    $tmp = search_config($this->config->data,"faiManagement","CLASS");
    if(!empty($tmp)){
      $this->fai_activated = TRUE;    
    }else{
      return;
    }

    $ui = get_userinfo();
    $tmp= get_permissions ($this->dn, $ui->subtreeACL);
    $this->acl= get_module_permission($tmp, "servrepository", $this->dn);
    
    $this->repositories = array();
    if(isset($this->attrs['FAIrepository'])){
      for($i = 0; $i < $this->attrs['FAIrepository']['count']; $i++){
        $tmp = split("\|",$this->attrs['FAIrepository'][$i]);
        $tmp2 = array();  
        $tmp3 = array();   

        if(isset($tmp[1])){
          $tmp2['ParentServer'] = $tmp[1];
          if(empty($tmp[1])){
            $tmp2['ParentServer'] = "none";
          }
        }else{
          $tmp2['ParentServer'] = "none";
        }

        if(isset($tmp[0])){
          $tmp2['Url']          = $tmp[0];
        }else{
          $tmp2['Url']          = "";
        }
  
        if(isset($tmp[2])){
          $tmp2['Release']      = $tmp[2];
        }else{
          $tmp2['Release']      = "";
        }

        if(isset($tmp[3])){
          $tmp3 = split(",",$tmp[3]);
          foreach($tmp3 as $sec){
            $tmp2['Sections'][$sec]=$sec;
          }    
        }else{
          $tmp['Section']=array();
        }

        $this->repositories[$tmp[2]]=$tmp2;      
      }
    }
  }

  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    if(!$this->fai_activated){
      $str = "<h2>"._("You can't use this plugin until FAI is activated.")."</h2>";
      return $str;
    }

    /* Fill templating stuff */
    $smarty= get_smarty();
    $display= "";

    $ui = get_userinfo();
    $tmp= get_permissions ($this->dn, $ui->subtreeACL);
    $this->acl= get_module_permission($tmp, "servrepository", $this->dn);
    $allow_edit = !preg_match("/disabled/i",chkacl($this->acl,"FAIclass"));
    

    /* Smarty vars*/
    $smarty->assign("infoimage",    get_template_path('images/info.png'));
    $smarty->assign("search_image", get_template_path('images/search.png'));
    $smarty->assign("launchimage",  get_template_path('images/small_filter.png'));
    $smarty->assign("tree_image",   get_template_path('images/tree.png'));
    $smarty->assign("alphabet",     generate_alphabet());
    $smarty->assign("apply",        apply_filter());
    $smarty->assign("regex",        $this->regex);


    /* Do we need to flip is_account state? */
    if (isset($_POST['modify_state'])){

      /* Only change account state if allowed */
      if($this->is_account && $this->acl == "#all#"){
        $this->is_account= !$this->is_account;
        $this->is_modified = true;
      }elseif(!$this->is_account && chkacl($this->acl,"create") == ""){
        $this->is_account= !$this->is_account;
        $this->is_modified = true;
      }
    }

    /* Show tab dialog headers */
    if ($this->is_account){
      $display= $this->show_header(_("Remove FAI repository extension."),
          _("This server has FAI repository features enabled. You can disable them by clicking below."));
    } else {
      $display= $this->show_header(_("Add FAI repository extension."),
          _("This server has FAI repository features disabled. You can enable them by clicking below."));
      return ($display);
    }

    /*
       ADD / EDIT Repository
       Dialog Handling
     */

    $once = false;
    if(isset($_POST['servRepository'])){
      foreach($_POST as $name => $value){

        if(preg_match("/AddRepository/",$name) && !$once && $allow_edit){
          $once = true;
          $this->dialog = new servRepositorySetup($this->config,$this->dn);
          $this->dialog->acl = $this->acl;
        }

        if((preg_match("/^delete_/",$name))&&(!$once) && $allow_edit){
          $once = true;
          $value = preg_replace("/delete_/","",$name);
          $value = base64_decode(preg_replace("/_.*$/","",$value));

          $url = $this->repositories[$value]['Url'];
          $release = $this->repositories[$value]['Release'];

          $ldap = $this->config->get_ldap_link();
          $ldap->cd ($this->config->current['BASE']);

          $ldap->search("(&(objectClass=gotoWorkstation)(objectClass=FAIobject)(FAIdebianMirror=".$url."))",array("cn","FAIclass"));

          $found = false;
          $found_in = " ";
          while($attrs = $ldap->fetch()){
            foreach($attrs['FAIclass'] as $class){
              if(preg_match("/".str_replace("/","\/",$release)."$/i",$class)){
                $found = true;  
                $found_in .= $attrs['cn'][0]." ";
              }
            }
          }

          if($found){
            print_red(sprintf(_("You can't delete this release, it is still used by these workstations [%s]. Please solve this dependencies first, to keep data base consistency."),$found_in));
          }else{
            if(isset($this->repositories[$value])){
              unset($this->repositories[$value]);
            }
          }
        }

        if((preg_match("/^edit_/",$name))&&(!$once) && $allow_edit){
          $value = preg_replace("/edit_/","",$name);
          $value = base64_decode(preg_replace("/_.$/","",$value));

          if(isset($this->repositories[$value])){

            $ldap = $this->config->get_ldap_link();
            $ldap->cd ($this->config->current['BASE']);

            $url = $this->repositories[$value]['Url'];
            $release = $this->repositories[$value]['Release'];

            $ldap->search("(&(objectClass=gotoWorkstation)(objectClass=FAIobject)(FAIdebianMirror=".$url."))",array("cn","FAIclass"));

            $found = false;
            $found_in = " ";
            while($attrs = $ldap->fetch()){
              foreach($attrs['FAIclass'] as $class){
                if(preg_match("/".str_replace("/","\/",$release)."$/i",$class)){
                  $found = true;
                  $found_in .= $attrs['cn'][0]." ";
                }
              }
            }

            if($found){
              print_red(sprintf(_("Be careful editing this release, it is still used by these workstations [%s]."),$found_in));
            } 

            if(isset($this->repositories[$value])){
              $once = true;
              $obj = $this->repositories[$value];

              /* to be able to detect if this was renamed */
              $obj['initialy_was'] = $obj['Release'];
              $this->dialog = new servRepositorySetup($this->config,$this->dn,$obj);
              $this->dialog->acl = $this->acl;
            }
          }
        }
      }
    }
    if((isset($_GET['act']))&&($_GET['act']=="open")&&(isset($_GET['id'])) && $allow_edit){
      $obj = $this->repositories[base64_decode($_GET['id'])];
      $obj['initialy_was'] = $obj['Release'];
      $this->dialog = new servRepositorySetup($this->config,$this->dn,$obj);
      $this->dialog->acl = $this->acl;
    }

    /*
    if(isset($_POST['AddRepository'])){
      $this->dialog = new servRepositorySetup($this->config,$this->dn);
      $this->dialog->acl = $this->acl;
    }
    */

    if(isset($_POST['repository_setup_save'])){
      $this->dialog->save_object();
      if(($this->dialog->is_new_name())&&(isset($this->repositories[$this->dialog->GetName()]))){
        print_red(_("This name is already in use."));
      }else

      if(count($this->dialog->check())!=0){
        foreach($this->dialog->check() as $msg){
          print_red($msg);
        }
      }else{
        $obj = $this->dialog->save();
        if($this->dialog->is_new_name()){
          $oldname = $this->dialog->initialy_was;
          $this->repositories[$obj['Release']]=$obj;        
          unset($this->repositories[$oldname]);
        }else{ 
          $this->repositories[$obj['Release']]=$obj;        
        }
        $this->dialog = NULL;
        $this->is_dialog= false;
      }
    }

    if(isset($_POST['repository_setup_cancel'])){
      $this->dialog=NULL;
      $this->is_dialog = false;
    }
   
    if($this->dialog != NULL){
      $this->dialog->save_object();
      $this->is_dialog = true;
      return($this->dialog->execute());
    }

    /*
      Repository setup dialog handling /END
    */

    $divlist = new divList("repositories");
    $divlist->SetEntriesPerPage(0);
    $divlist->setHeight(400);
    $divlist->SetPluginMode();

    $divlist->setHeader(array(array("string"=>_("Release"),"attach"=>"style='width:80px;'"),
                              array("string"=>_("Sections")),
                              array("string"=>_("Options"),"attach"=>"style='border-right:0px;width:55px;'")
                       )     );

    $link   = "<a href='?plug=".$_GET['plug']."&amp;act=open&amp;id=%s'>%s</a>";
    $edit   = "<input type='image' value='%s' name='edit_%s'   src='images/edit.png'>&nbsp;";
    $delete = "<input type='image' value='%s' name='delete_%s' src='images/edittrash.png'>";
  
    foreach($this->repositories as $name => $reps){

      $str = " ";
      if(preg_match("/".str_replace("*",".*",$this->regex)."/",$reps['Release'])){

      foreach($reps['Sections'] as $sec){
        $str.=$sec." ";  
      }    

      if($allow_edit){
        $link_str = sprintf($link,base64_encode($name),$name);
        $sections = sprintf($link,base64_encode($name),_("Sections")." :".$str);
        $options  = preg_replace("/%s/",base64_encode($name),$edit.$delete);
      }else{
        $link_str = $name;
        $sections = _("Sections")." :".$str;
        $options  = "";
      }
    
 
      $divlist->AddEntry(array(
                              array("string"=>$link_str,"attach"=>"style='width:80px;'"),
                              array("string"=>$sections),
                              array("string"=>$options,"attach"=>"style='border-right:0px;width:55px;text-align:right;'")));
      }
    }
 
    $smarty -> assign("Repositories",$divlist->DrawList());
    $smarty -> assign("FAIclassACL",chkacl($this->acl,"FAIclass"));

    $display.= $smarty->fetch(get_template_path('servRepository.tpl', TRUE));
    return($display);
  }

  function remove_from_parent()
  {

    if(!$this->fai_activated) return;
    
    /* Skip if not allowed */ 
    $ui = get_userinfo();
    $tmp= get_permissions ($this->dn, $ui->subtreeACL);
    $this->acl= get_module_permission($tmp, "servrepository", $this->dn);

    if(preg_match("/disabled/",chkacl($this->acl,"FAIclass"))) return;

    plugin::remove_from_parent();    
    $ldap= $this->config->get_ldap_link();
    $ldap->cd ($this->config->current['BASE']);
    
    $ldap->cat($this->dn, array('dn'));
    
    if($ldap->count()){
      $ldap->cd($this->dn);
      $this->cleanup();
      $ldap->modify ($this->attrs); 

      $this->handle_post_events("modify");
    }
  }


  /* Save data to object */
  function save_object()
  {
    plugin::save_object();
    if(isset($_POST['regex'])){
      $this->regex = $_POST['regex'];
    }
  }


  /* Check supplied data */
  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();
        
    return ($message);
  }


  /* Save to LDAP */
  function save()
  {
    if(!$this->fai_activated) return;

    /* Skip if not allowed */ 
    $ui = get_userinfo();
    $tmp= get_permissions ($this->dn, $ui->subtreeACL);
    $this->acl= get_module_permission($tmp, "servrepository", $this->dn);

    if(preg_match("/disabled/",chkacl($this->acl,"FAIclass"))) return;
    plugin::save();

    $arr = array();
    foreach($this->repositories as $servername => $conf){
      $str = "";
      foreach($conf['Sections'] as $sec){
        $str.=$sec.",";
      }
      $str=preg_replace("/,$/","",$str);
        
      if($conf['ParentServer']=="none"){
        $conf['ParentServer'] ="";
      }    
    
      $arr[]=$conf['Url']."|".$conf['ParentServer']."|".$conf['Release']."|".$str;
    }
    $this->attrs['FAIrepository'] = $arr;

    $ldap= $this->config->get_ldap_link();
    $ldap->cd ($this->config->current['BASE']);
    
    $ldap->cat($this->dn, array('dn'));
    
    if($ldap->count()){
      $ldap->cd($this->dn);
      $this->cleanup();
      $ldap->modify ($this->attrs);       

      $this->handle_post_events("modify");
    }else{
      $ldap->cd ($this->config->current['BASE']);
      $ldap->create_missing_trees($this->dn);
      $ldap->cd($this->dn);
      $ldap->add($this->attrs);
      $this->handle_post_events("add");
    }
  }

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>