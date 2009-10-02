<?php

class printerPPDSelectionDialog extends plugin
{
  /* attribute list for save action */
  var $ignore_account       = FALSE;
  var $attributes           = array();
  var $objectclasses        = array("whatever");

  var $list                 =array();
  var $header               =array();
  var $current_del_id       = "";
  var $Vendor               = "";

  function printerPPDSelectionDialog (&$config, $dn= NULL,$list=false,$headers=false,$ppd=false)
  {
    plugin::plugin ($config, $dn);
    $this->list       = $list;
    $this->header     = $headers;
    $this->depselect  = $this->config->current['BASE'];
    
    if(!session::is_set('printerPPDSelectionDialog')){
      session::set("printerPPDSelectionDialog",array('regex'=> "*"));
    }  

    /* Order the manufacturers index */
    ksort($this->header);
  }

  function execute()
  {
	/* Call parent execute */
	plugin::execute();

    /* Fill templating stuff */
    $smarty= get_smarty();
    $display= "";
    $s_action = "none";
 
    $printerPPDSelectionDialog = session::get('printerPPDSelectionDialog');
    $regex = $printerPPDSelectionDialog['regex'];

    if(isset($_GET['search'])){
      $regex = $_GET['search']."*";
    }
  
    if(isset($_POST['regex'])){
      $regex= $_POST['regex'];
    }
    
    $regex = str_replace("**","*",$regex);
    $printerPPDSelectionDialog['regex'] = $regex;
    session::set('printerPPDSelectionDialog',$printerPPDSelectionDialog);
 

    /* Delete requested, check if everything is ok and display confirmation dialog */
    if(isset($_GET['act']) && $_GET['act'] == "del"){

      /* Get id and check if id is valid */
      $id_to_del = base64_decode($_GET['id']);
      $found = "";
      foreach($this->list[$this->Vendor] as $key => $data){
        if($data['link'] == $id_to_del){
          $found = $key;
          break;
        }
      }
      if(!empty($found)){
        $this->current_del_id = $id_to_del;

        $smarty->assign("warning", sprintf(msgPool::deleteInfo($found)));
        return($smarty->fetch(get_template_path('remove_ppd.tpl', TRUE)));

      }else{
        msg_dialog::display(_("PPD error"), msgPool::fileDoesNotExist($id_to_del), ERROR_DIALOG);
      }
    }

    /* Deletion confirmed */
    if(isset($_POST['delete_ppd_confirm']) && !empty($this->current_del_id)){

      /* check if file is removeable */
      $is_ok = false;
      if(is_writeable($this->current_del_id)){
        $is_ok = @unlink($this->current_del_id);

        /* Update ppd list */
        if($is_ok){
          foreach($this->list as $vendor => $ppds){
            foreach($ppds as $ppd => $data){
              if($data['link'] == $this->current_del_id){
                unset($this->list[$vendor][$ppd]);
              }
            }
          }
        }
      }

      if(!$is_ok){
        msg_dialog::display(_("PPD error"), msgPool::cannotDeleteFile($this->current_del_id), ERROR_DIALOG);
      }
    }


    /** Added **/
    $list = array();
    foreach($this->list as $cat => $ppds){
      foreach($ppds as $ppd){
      	$name = $ppd['ppd']['name'];
        if(preg_match("/^".str_replace("*",".*",$regex)."/i",$name)){
          if(is_readable($ppd['link'])){
            $list[$ppd['link']] = $ppd;
          }
        }
      }
    }
    
    /* Sort the list alphabetically */
    asort($list);
 
    if((isset($_GET['act']))&&($_GET['act']=="open"))  {
      
      if((!empty($_GET['id'])) && (!isset( $this->header[base64_decode($_GET['id'])]))){
        msg_dialog::display(_("PPD error"), sprintf(_("Cannot locate vendor '%s' in available PPDs!"), base64_decode($_GET['id'])), ERROR_DIALOG);
        $this->Vendor = ""; 
      }else{
        $this->Vendor = base64_decode($_GET['id']);
      }
    }

    $div = new divSelectBox("printerPPDSelectionDialog");
    $div ->SetHeight(450);
    $div ->SetSummary(_("Printer ppd selection."));
    
    $linkopen = "<a href='?plug=".$_GET['plug']."&amp;act=open&amp;id=%s'>
                  <img src='images/lists/folder.png' class='center' border=0>&nbsp;%s
                 </a>";
    $uselink  = "<a href='?plug=".$_GET['plug']."&amp;act=use&amp;id=%s'>%s</a>";
    $dellink  = "<a href='?plug=".$_GET['plug']."&amp;act=del&amp;id=%s'>
                  <img src='images/lists/trash.png' alt='"._("delete")."' border=0>
                 </a>"; 

    if(empty($this->Vendor)){
      foreach($this-> header as $key => $entry){
        $div ->AddEntry (array(
        	  
              array("string"=>sprintf($linkopen,base64_encode($key),$key),"attach"=>"style='border-right:0px;'")
              ));
      }
    }else{
      $div ->AddEntry (array(
            array("string"=>sprintf($linkopen,"",".. ["._("back")."]"),"attach"=>"style='border-right:0px;'")
            ));
      foreach($list as $key => $ppd){
      	$name = $ppd['ppd']['name'];
        if(preg_match("/^".$this->Vendor."/", $name)){

          if(is_writeable($ppd['link'])){
            $del_str = sprintf($dellink,base64_encode($key));
          }else{
            $del_str = "";
          }

          $div ->AddEntry (array(
                                  array("string"=>sprintf($uselink,base64_encode($key),$name)),
                                  array("string"=>$del_str,"attach"=>"style='border-right:0px;'")

                ));
        }
      }
    }

    $smarty->assign("List",         $div -> DrawList());
    $smarty->assign("search_image", get_template_path('images/lists/search.png'));
    $smarty->assign("launchimage",  get_template_path('images/lists/action.png'));
    $smarty->assign("tree_image",   get_template_path('images/lists/search-subtree.png'));
    $smarty->assign("alphabet",     generate_alphabet());
    $smarty->assign("apply",        apply_filter());
    $smarty->assign("regex",        $regex);
  
    $display.= $smarty->fetch(get_template_path('printerPPDSelectionDialog.tpl', TRUE,dirname(__FILE__)));
    return($display);
  }

  function save_object()
  {
  } 

  function check(){
  }

  /* Save to LDAP */
  function save()
  {
    return $this->selectedPPD;
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>