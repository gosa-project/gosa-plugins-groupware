<?php

class filterSYSTEMS {

  static function query($parent,$base, $scope, $filter, $attributes, $category, $objectStorage= array())
  {
    $ui = get_userinfo();
    global $config;

    // Check whether the arp handling active or not
    $arp_handling_active = ($config->search("ArpNewDevice","CLASS",array('tabs')) != "");

    $config= session::global_get('config');
    $ldap= $config->get_ldap_link(TRUE);
    $flag= ($scope == "sub")?GL_SUBSEARCH:0;
    $entries = filterSYSTEMS::get_list($parent,$base, $filter, $attributes, $category, $objectStorage, $flag | GL_SIZELIMIT);

    foreach($entries as $key => $entry){
      if(preg_match("/".preg_quote(get_ou('systemIncomingRDN'),'/')."/i", $entry['dn'])){

        // Detect incoming types
        if(in_array('gotoWorkstation', $entry['objectClass'])){
          $entries[$key]['objectClass'][] = 'FAKE_OC_NewWorkstation';
          $entries[$key]['cn'][0].= " <i>("._("New workstation").")</i>";
        }elseif(in_array('gotoTerminal', $entry['objectClass'])){
          $entries[$key]['objectClass'][] = 'FAKE_OC_NewTerminal';
          $entries[$key]['cn'][0].= " <i>("._("New terminal").")</i>";
        }elseif($arp_handling_active && 
            in_array('GOhard', $entry['objectClass']) && 
            !isset($entry['gotoMode'][0])){ 
          $entries[$key]['objectClass'][] = 'FAKE_OC_ArpNewDevice';
          $entries[$key]['cn'][0].= " <i>("._("New unknown device").")</i>";
        }elseif(in_array('GOhard', $entry['objectClass'])){
          $entries[$key]['objectClass'][] = 'FAKE_OC_NewDevice';
          $entries[$key]['cn'][0].= " <i>("._("New device").")</i>";
        }

        // Detect SAMBA machines
      }elseif(in_array('sambaSamAccount', $entry['objectClass']) && preg_match("/\$$/", $entry['cn'][0])){
        $entries[$key]['objectClass'][] = 'sambaSamAccount';

        // Detect Workstation & templates
      }elseif(in_array('gotoWorkstation', $entry['objectClass'])){
        if(!isset($entry['macAddress'][0]) || $entry['macAddress'][0] == "-"){
          continue; // We do not need templates anymore, we've ogroups now.
          //$entries[$key]['objectClass'][] = 'FAKE_OC_WorkstationTemplate';
        }
        $pwd_acl = $ui->get_permissions($entry['dn'],'workstation/workgeneric','userPassword');
        if(preg_match("/w/",$pwd_acl)){
          $entries[$key]['objectClass'][] = 'FAKE_OC_PWD_changeAble';
        }

        // Detect Terminal & templates
      }elseif(in_array('gotoTerminal', $entry['objectClass'])){
        if(!isset($entry['macAddress'][0]) || $entry['macAddress'][0] == "-"){
          continue; // We do not need templates anymore, we've ogroups now.
          //$entries[$key]['objectClass'][] = 'FAKE_OC_TerminalTemplate';
        }
        $pwd_acl = $ui->get_permissions($entry['dn'],'terminal/termgeneric','userPassword');
        if(preg_match("/w/",$pwd_acl)){
          $entries[$key]['objectClass'][] = 'FAKE_OC_PWD_changeAble';
        }

        // Detect Server & templates
      }elseif(in_array('goServer', $entry['objectClass'])){
        if(!isset($entry['macAddress'][0]) || $entry['macAddress'][0] == "-"){
          continue; // We do not need templates anymore, we've ogroups now.
          //$entries[$key]['objectClass'][] = 'FAKE_OC_ServerTemplate';
        }
        $pwd_acl = $ui->get_permissions($entry['dn'],'server/servgeneric','userPassword');
        if(preg_match("/w/",$pwd_acl)){
          $entries[$key]['objectClass'][] = 'FAKE_OC_PWD_changeAble';
        }

        // Detect ieee802Device
      }elseif(in_array('ieee802Device', $entry['objectClass'])){
        $pwd_acl = $ui->get_permissions($entry['dn'],'component/componentGeneric','userPassword');
        if(preg_match("/w/",$pwd_acl)){
          $entries[$key]['objectClass'][] = 'FAKE_OC_PWD_changeAble';
        }
      }

      // Detect object states
      foreach(array('gotoWorkstation','gotoTerminal','goServer') as $type){
        if(in_array($type, $entry['objectClass'])){

          if(isset($entry['gotoMode'][0]) && preg_match("/locked/i", $entry['gotoMode'][0])){
            $entries[$key]['objectClass'][] = $type."__IS_LOCKED";
          }elseif(isset($entry['FAIstate'][0])){
            switch (preg_replace('/:.*$/', '', $entry['FAIstate'][0])) {
              case 'error':
                $state= '__IS_ERROR';
                break;
              case 'installing':
              case 'install':
              case 'sysinfo':
              case 'softupdate':
              case 'scheduledupdate':
                $state= '__IS_BUSY';
                break;
              default: $state ="";
            }
            if(!empty($state)) $entries[$key]['objectClass'][] = $type.$state;
          }
        }
      }
    }

    // Append opsi stuff ...
    global $config;
    $ui = get_userinfo();
    if(preg_match("/FAIstate=__NETBOOT__/", $filter) && 
        class_available('opsi') &&
        $base == $config->current['BASE']){

      // Get filter regex ...
      $regex = "";
      if(preg_match("/\(cn=[^\)\$]*\)/", $filter)){
        $regex = trim(preg_replace("/^.*\(cn=([^\)\$]*)\).*$/","\\1", $filter),"* ");
      }

      // Check ACLs and opsi handle
      $opsi_acl = $ui->get_permissions($base,"opsi/opsiProperties");
      $opsi = new opsi($config);
      $objects = array();
      if($opsi instanceof opsi && $opsi->enabled() && preg_match("/r/",$opsi_acl)){

        // Get list of opsi hosts 
        $opsi_clients = $opsi->get_hosts_for_system_management();
        if($opsi->is_error()){
          msg_dialog::display(_("Error"),msgPool::siError($opsi->get_error()),ERROR_DIALOG);
        }else{
          foreach($opsi_clients as $entry){
            if(empty($regex) || preg_match('/'.preg_quote($regex,'/').'/i',$entry['cn'][0])){
              $entry['objectClass'] = array("FAKE_OC_OpsiHost");
              $item = array('count' => count($entry));
              foreach($entry as  $name => $value){
                $item[] = $name;
                $item[$name] = $value;
              }
              $entries[] = $item;
            }
          }
        }
      }
    }
    return($entries);
  }


  static function get_list($parent,$base, $filter, $attributes, $category, $objectStorage, $flags= GL_SUBSEARCH)
  {
    $ui= session::global_get('ui');
    $config= session::global_get('config');

    // Move to arrays for category and objectStorage
    if (!is_array($category)) {
      $category= array($category);
    }

    // Store in base - i.e. is a rdn value empty?
    $storeOnBase= count($objectStorage) == 1 && empty($objectStorage[0]);

    $method= ($storeOnBase && !($flags & GL_SUBSEARCH))?"ls":"search";

    // Initialize search bases
    $bases= array();

    // Get list of sub bases to search on
    if ($storeOnBase) {
      $bases[$base]= "";
    } else {
      foreach ($objectStorage as $oc) {
        $oc= preg_replace('/,$/', '', $oc);
        $tmp= explode(',', $oc);
        if (count($tmp) == 1) {
          preg_match('/([^=]+)=(.*)$/', $oc, $m);
          if ($flags & GL_SUBSEARCH) {
            $bases[$base][]= $m[1].":dn:=".$m[2];
          } else {
            $bases["$oc,$base"][]= $m[1].":dn:=".$m[2];
          }
        } else {
          // No, there's no \, in pre defined RDN values
          preg_match('/^([^,]+),(.*)$/', $oc, $matches);
          preg_match('/([^=]+)=(.*)$/', $matches[1], $m);
          if ($flags & GL_SUBSEARCH) {
            $bases[$base][]= $m[1].":dn:=".$m[2];
          } else {
            $bases[$matches[2].",$base"][]= $m[1].":dn:=".$m[2];
          }
        }
      }
    }
    // Get LDAP link
    $ldap= $config->get_ldap_link($flags & GL_SIZELIMIT);

    // Do search for every base
    $result= array();
    $limit_exceeded = FALSE;
    foreach($bases as $base => $dnFilters) {

      // Break if the size limit is exceeded
      if($limit_exceeded){
        return($result);
      }

      // Switch to new base and search
      if (is_array($dnFilters)){
        $dnFilter= "(|";
        foreach ($dnFilters as $df) {
          $dnFilter.= "($df)";
        }
        $dnFilter.= ")";
      } else {
        $dnFilter= "";
      }
      $ldap->cd($base);
      if ($method == "ls") {
        $ldap->ls("(&$filter$dnFilter)", $base, $attributes);
      } else {
        $ldap->search("(&$filter$dnFilter)", $attributes);
      }

      // Check for size limit exceeded messages for GUI feedback
      if (preg_match("/size limit/i", $ldap->get_error())){
        session::set('limit_exceeded', TRUE);
        $limit_exceeded = TRUE;
      }

      /* Crawl through result entries and perform the migration to the
         result array */
      while($attrs = $ldap->fetch()) {
        $dn= $ldap->getDN();

        /* Convert dn into a printable format */
        if ($flags & GL_CONVERT){
          $attrs["dn"]= convert_department_dn($dn);
        } else {
          $attrs["dn"]= $dn;
        }

        /* Skip ACL checks if we are forced to skip those checks */
        if($flags & GL_NO_ACL_CHECK){
          $result[]= $attrs;
        }else{

          if(preg_match("/".preg_quote(get_ou('systemIncomingRDN'),'/')."/i",$dn)){
            $attrs['objectClass'][] = "gotoTerminal";
          }
          $obj = $parent->headpage->getObjectType($parent->headpage->objectTypes, $attrs['objectClass']);

          if(isset($obj['category'])){
            $o = $obj['category']."/".$obj['class'];
            if(preg_match("/r/",$ui->get_permissions($dn,$o))){
              $result[]= $attrs;
            }
          }
        }
      }

    }

    return $result;
  }
}

?>