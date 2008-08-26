<?php

class opsigeneric extends plugin
{
  private $opsi;
  private $hostId;  

  /* Contains a list of all available netboot products 
   */
  private $a_availableNetbootProducts = array();
  private $s_selectedNetbootProduct = "";  
  private $s_initial_selectedNetbootProduct = "";  

  /* Contains a list of all available local products
   */
  private $a_availableLocalProducts = array();
  private $a_selectedLocalProducts = array();
  private $a_initial_selectedLocalProducts = array();

  private $init_failed = FALSE;

  private $parent_mode = TRUE;
  private $is_installed = FALSE;

  public $mac          = "";
  public $note         = "";
  public $description  = "";

  public $attributes = array("mac","note","description");

  public function __construct($config,$hostId)
  {
    $this->opsi = new opsi($config); 
    $this->is_account =TRUE;
  
    /* Get hostId */
    if($hostId != "new"){
      $this->initially_was_account = TRUE;
      $this->hostId = preg_replace("/^opsi:=([^,]*),.*$/","\\1",$hostId);
    }
  
    /* Try to plugin */
    $this->init();
  }
  
  private function init()
  {
    $err = FALSE;
    $this->init_failed = FALSE;

    if($this->hostId != "new"){
      $list = $this->opsi->list_clients($this->hostId);
      $err |= $this->opsi->is_error();
      foreach($list as $entry){
        if($entry['NAME'][0]['VALUE'] == $this->hostId){  
          foreach(array("description" => "DESCRIPTION","mac" => "MAC", "note" => "NOTES") as $des => $src){
            $this->$des = $entry[$src][0]['VALUE'];
          } 
          break;
        }
      }
    }

    /* Get product settings */     
    if(!$err){
      $this->a_availableNetbootProducts = $this->opsi->get_netboot_products();
      $err |= $this->opsi->is_error();
    }
    if(!$err) {
      $this->a_availableLocalProducts   = $this->opsi->get_local_products();
      $err |= $this->opsi->is_error();
    }

    /* Get selected products */
    if(!$err && !empty($this->hostId)) {
      $tmp = array_keys($this->opsi->get_netboot_products($this->hostId));
      if(count($tmp)){
        $this->s_selectedNetbootProduct = $tmp[0];
      }
      $err |= $this->opsi->is_error();
    }
    if(!$err && !empty($this->hostId)) {
      $tmp = $this->opsi->get_local_products($this->hostId); 
      $err |= $this->opsi->is_error();
      $this->a_selectedLocalProducts = $tmp;
    }

    /* Load product configuration */
    if(!$err && !empty($this->hostId)) {
      foreach($this->a_selectedLocalProducts as $name => $data){
        $CFG = $this->opsi->get_product_properties($name,$this->hostId);
        $err |= $this->opsi->is_error();
        $this->a_selectedLocalProducts[$name]['CFG'] = $CFG;
      }
    }
  
    /* Check if everything went fine else reset everything and display a retry button 
     */
    if($err){
      $this->a_availableNetbootProducts = array();
      $this->s_selectedNetbootProduct = "";  
      $this->s_initial_selectedNetbootProduct = "";  
      $this->a_availableLocalProducts = array();
      $this->a_selectedLocalProducts = array();
      $this->a_initial_selectedLocalProducts = array();
      $this->init_failed = TRUE;
    }else{

      /* Remember initial settings */ 
      $this->a_initial_selectedLocalProducts = $this->a_selectedLocalProducts;
      $this->s_initial_selectedNetbootProduct = $this->s_selectedNetbootProduct;
    }
  }

  public function check()
  {
    $messages = plugin::check();
    if(!preg_match("/\./",$this->hostId)){
      $messages[] = msgPool::invalid(_("Name"),$this->hostId,"",_("The client name must contain a domain part (e.g. '.company.de')."));
    }
    if(!tests::is_mac($this->mac) || empty($this->mac)){
      $messages[] = msgPool::invalid(_("MAC address"),$this->mac,"","00:0C:7F:31:33:F1");
    }
    return($messages);
  }

  public function execute()
  {
    if($this->init_failed){
      $smarty = get_smarty();
      $smarty->assign("init_failed",TRUE);
      $smarty->assign("message",$this->opsi->get_error());
      return($smarty->fetch(get_template_path("generic.tpl",TRUE,dirname(__FILE__))));
    }  

    if(is_object($this->dialog)){
      $this->dialog->save_object();
      return($this->dialog->execute());
    }

    $smarty = get_smarty();

    foreach($this->attributes as $attr){
      $smarty->assign($attr,$this->$attr);
    }

    $smarty->assign("parent_mode", $this->parent_mode);
    $smarty->assign("is_installed", $this->is_installed);
    $smarty->assign("init_failed",FALSE);
    $divSLP = new divSelectBox();
    $divALP = new divSelectBox();

    /* Create list of available local products 
     */
    ksort($this->a_availableLocalProducts);
    foreach($this->a_availableLocalProducts as $name => $data){
      if(isset($this->a_selectedLocalProducts[$name])) continue;

      $add_tab  = array("string"   => "<input type='image' src='images/back.png' name='add_lp_".$name."'>");
      $name_tab = array("string"   => $name);
      $desc_tab = array("string"   => "<div style='height: 14px;overflow:hidden;'>".$data['DESC']."</div>",
          "attach"   => "title='".$data['DESC']."' style='border-right:0px;'");
      $divALP->AddEntry(array($add_tab,$name_tab,$desc_tab));
    }

    /* Create list of selected local products 
     */
    ksort($this->a_selectedLocalProducts);
    foreach($this->a_selectedLocalProducts as $name => $data){

      $name_tab = array("string"   => $name);
      $desc_tab = array(
          "string" => "<div style='height: 14px;overflow:hidden;'>".$data['DESC']."</div>",
          "attach" => "title='".$data['DESC']."'");

      /* Only display edit button, if there is something to edit 
       */
      $edit = "<img src='images/empty.png' alt=' '>";
      if(count($data['CFG'])){
        $edit = "<input type='image' src='images/lists/edit.png' name='edit_lp_".$name."'>";
      }
      $del  = "<input type='image' src='images/lists/trash.png' name='del_lp_".$name."'>";  

      $opt_tab  = array("string" => $edit.$del,
          "attach" => "style='border-right:0px; width: 40px; text-align:right;'");
      $divSLP->AddEntry(array($name_tab,$desc_tab,$opt_tab));
    }

    ksort($this->a_availableNetbootProducts);
    $smarty->assign("hostId", $this->hostId);
    $smarty->assign("divSLP", $divSLP->DrawList());
    $smarty->assign("divALP", $divALP->DrawList());
    $smarty->assign("SNP", $this->s_selectedNetbootProduct);
    $smarty->assign("ANP", $this->a_availableNetbootProducts);
    return($smarty->fetch(get_template_path("generic.tpl",TRUE,dirname(__FILE__))));
  }


  public function save()
  {
    /* Check if this a new opsi client 
        -Do we have to create this client first?
     */
    if(!$this->initially_was_account && $this->is_account){
      $res = $this->opsi->add_client($this->hostId,$this->mac,$this->note,$this->description);
      if($this->opsi->is_error()){
        msg_dialog::display(_("Error"),msgPool::siError($this->opsi->get_error()),ERROR_DIALOG);    
        return;
      }
    }
    $this->opsi->modify_client($this->hostId,$this->mac,$this->note,$this->description);
    if($this->opsi->is_error()){
      msg_dialog::display(_("Error"),msgPool::siError($this->opsi->get_error()),ERROR_DIALOG);
      return;
    }

    $add = array_diff_assoc($this->a_selectedLocalProducts,$this->a_initial_selectedLocalProducts);
    $del = array_diff_assoc($this->a_initial_selectedLocalProducts,$this->a_selectedLocalProducts);

    foreach($del as $name => $data){
      $this->opsi->del_product_from_client($name,$this->hostId);
      if($this->opsi->is_error()){
        msg_dialog::display(_("Error"),msgPool::siError($this->opsi->get_error()),ERROR_DIALOG);    
        return;
      }
    }
    foreach($add as $name => $data){
      $this->opsi->add_product_to_client($name,$this->hostId);
      if($this->opsi->is_error()){
        msg_dialog::display(_("Error"),msgPool::siError($this->opsi->get_error()),ERROR_DIALOG);    
        return;
      }
      $this->opsi->set_product_properties($name,$data['CFG'],$this->hostId);
      if($this->opsi->is_error()){
        msg_dialog::display(_("Error"),msgPool::siError($this->opsi->get_error()),ERROR_DIALOG);    
        return;
      }
    }

    foreach($this->a_selectedLocalProducts as $name => $data){
      if(isset($del[$name]) || isset($add[$name])) continue;
      $diff = array_diff($data['CFG'],$this->a_initial_selectedLocalProducts[$name]['CFG']);
      if(count($diff)){
        $this->opsi->set_product_properties($name,$diff,$this->hostId);
        if($this->opsi->is_error()){
          msg_dialog::display(_("Error"),msgPool::siError($this->opsi->get_error()),ERROR_DIALOG);    
          return;
        }
      }
    }

    if($this->s_selectedNetbootProduct != $this->s_initial_selectedNetbootProduct){
      if(!empty($this->s_initial_selectedNetbootProduct)){
        $this->opsi->del_product_from_client($this->s_initial_selectedNetbootProduct,$this->hostId);
        if($this->opsi->is_error()){
          msg_dialog::display(_("Error"),msgPool::siError($this->opsi->get_error()),ERROR_DIALOG);    
          return;
        }
      }
      $this->opsi->add_product_to_client($this->s_selectedNetbootProduct,$this->hostId);
      if($this->opsi->is_error()){
        msg_dialog::display(_("Error"),msgPool::siError($this->opsi->get_error()),ERROR_DIALOG);    
        return;
      }
    }
  }

  public function remove_from_parent()
  {
    $this->opsi->del_client($this->hostId);
    if($this->opsi->is_error()){
      msg_dialog::display(_("Error"),msgPool::siError($this->opsi->get_error()),ERROR_DIALOG);
      return;
    }
  }


  public function save_object()
  {
    if(isset($_POST['reinit']) && $this->init_failed){
      $this->init();
    }

    if(isset($_POST['cancel_properties']) && is_object($this->dialog)){
      $this->dialog = NULL;
    }
    if(isset($_POST['save_properties']) && ($this->dialog instanceof opsi_product_config)){
      $this->dialog->save_object();
      $pro = $this->dialog->get_product();
      $CFG = $this->dialog->get_cfg();
      if(isset($this->a_selectedLocalProducts[$pro])){
        $this->a_selectedLocalProducts[$pro]['CFG'] = $CFG;
      }
      $this->dialog = NULL;
    }

    if(isset($_POST['opsigeneric_posted'])){

      plugin::save_object();

      if(isset($_POST['hostId']) && $this->parent_mode){
        $this->hostId = get_post('hostId');
      }

      /* Send actions like 'install' or 'wake' to the si server 
       */
      if(isset($_POST['opsi_action']) && isset($_POST['opsi_trigger_action']) && $this->parent_mode){
        $action = $_POST['opsi_action'];
        if(in_array($action,array("wake","install"))){
          $this->opsi->send_action($action,$this->hostId,$this->mac);
          if($this->opsi->is_error()){
            msg_dialog::display(_("Error"),msgPool::siError($this->opsi->get_error()),ERROR_DIALOG);
          }
        }
      }

      if(isset($_POST['opsi_netboot_product'])){
        $SNP = trim($_POST['opsi_netboot_product']);
        if(isset($this->a_availableNetbootProducts[$SNP])){
          $this->s_selectedNetbootProduct = $SNP;
        }
      }

      foreach($_POST as $name => $value){
        if(preg_match("/^add_lp_/",$name)){
          $product = preg_replace("/^add_lp_(.*)_.$/","\\1",$name);
          if(isset($this->a_availableLocalProducts[$product]) && !isset($this->a_selectedLocalProducts[$product])){
            $this->a_selectedLocalProducts[$product] = $this->a_availableLocalProducts[$product];
            $CFG = $this->opsi->get_product_properties($product);
            $this->a_selectedLocalProducts[$product]['CFG'] = $CFG;
          }
          break;
        }
        if(preg_match("/^del_lp_/",$name)){
          $product = preg_replace("/^del_lp_(.*)_.$/","\\1",$name);
          if(isset($this->a_selectedLocalProducts[$product])){
            unset($this->a_selectedLocalProducts[$product]);
          }
          break;
        }
        if(preg_match("/^edit_lp_/",$name)){
          $product = preg_replace("/^edit_lp_(.*)_.$/","\\1",$name);
          $this->dialog = new opsi_product_config($this->config,
              $product,$this->a_selectedLocalProducts[$product]['CFG'],$this->hostId);
          break;
        }
      }   
    }
  }


  function install_client()
  {

  }


  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Generic"),
          "plDescription" => _("Opsi generic"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 1,
          "plSection"     => array("administration"),
          "plCategory"    => array("opsi" => array("description"  => _("Opsi client"),
                                                     "objectClass"  => "dummy_class_opsi")),
          "plProvidedAcls"=> array()
          ));
  }

}


// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>