<?php

class opsiLicenceHandler extends opsi  {



  /*******************
   * POOLs
   *******************/

  protected $use_alternative_xml_parse_method = TRUE;


  /* @brief   Returns licensePoolId, description, productIds and windowsSoftwareIds 
   *           for all found license pools.
   */
  function listPools()
  {
    $data= array();
    $res = $this->send_data("gosa_opsi_getLicensePools_listOfHashes",$this->target,$data,TRUE);
    
    $items  = array();
    if(isset($res['XML'][0]['RESULT'][0]['HIT'])){
      $items = $res['XML'][0]['RESULT'][0]['HIT'];
      $data =array();
      foreach($items as $item){
        $entry = array();
        foreach(
          array(
            "DESCRIPTION"       => "description",
            "LICENSEPOOLID"     => "cn",
            "PRODUCTIDS"        => "productId",
            "WINDOWSSOFTWAREIDS"=> "softwareId") as $source => $dest){
        
          if(isset($item[$source])){
            $entry[$dest] = array('count' => 0);
            foreach($item[$source] as $obj){
              $entry[$dest][] = $obj['VALUE'];
            }
            $entry[$dest]['count'] = (count($entry[$dest]) -1 );
          } 
        }
        $data[] =$entry;
      }
      return($data);
    }
    return(FALSE);
  }


  /* @brief Create a license pool at Opsi server.
   * @param licensePoolId The name of the pool (optional).
   * @param description The description of the pool (optional).
   * @param productIds A list of assigned porducts of the pool (optional).
   * @param windowsSoftwareIds A list of windows software Ids associated to the pool (optional).
   */
  function createPool($poolId,$desc = "",$products = array(),$softwareIds = array())
  {
    $data= array();
    $data['licensePoolId']        = htmlentities($poolId);
    if(!empty($desc)){
      $data['description']        = htmlentities($desc);
    }
    if(count($products)){
      $data['productIds']         = $products;
    }
    if(count($softwareIds)){
      $data['windowsSoftwareIds'] = $softwareIds;
    }

    $res = $this->send_data("gosa_opsi_createLicensePool",$this->target,$data,TRUE);
    if(isset($res['XML'][0]['ANSWER_OPSI_CREATELICENSEPOOL'])){
      return(TRUE);
    }
    return(FALSE);
  }


  /* 
   * @brief Return productIds, windowsSoftwareIds and description for a given licensePoolId
   * @param licensePoolId The name of the pool.
   */
  function getPool($licensePoolId)
  {
    $data= array();
    $data['licensePoolId'] = htmlentities($licensePoolId);
    $res = $this->send_data("gosa_opsi_getLicensePool_hash",$this->target,$data,TRUE);
    
    print_a($res);

    if(isset($res['XML'][0]['ANSWER_OPSI_GETLICENSEPOOL_HASH'])){
      $item = array();
      foreach(array("LICENSEPOOLID"       => "licensePoolId", 
                    "DESCRIPTION"         => "description",
                    "WINDOWSSOFTWAREIDS"  => "windowsSoftwareIds",
                    "PRODUCTIDS"          => "productIds") as $source => $target){
        if(isset($res['XML'][0][$source])){
          foreach($res['XML'][0][$source] as $data){
            $item[$target][] = $res['XML'][0][$source][0]['VALUE'];
          }
        }
      }
      return($item);
    }
    return(FALSE);
  }


  /*
   * @brief 
   *    Delete licnese pool by license pool Id. 
   *    A pool can only be deleted if there are no software licenses bound to the pool.
   *    The fixed parameter deleteLicenses=True specifies that 
   *      all software licenses bound to the pool are being deleted.
   * @param licensePoolId The name of the pool.
   */
  function deletePool($poolId, $force = FALSE)
  {
    $data= array();
    $data['licensePoolId']        = htmlentities($poolId);
    if($force){
      $data['deleteLicenses']     = 'TRUE';
    }
    $res = $this->send_data("gosa_opsi_deleteLicensePool",$this->target,$data,TRUE);
    if(isset($res['XML'][0]['ANSWER_OPSI_DELETELICENSEPOOL'])){
      return(TRUE);
    }
    return(FALSE);
  }


  /*******************
   * LICENSES
   *******************/

  /*  
   * @brief 
   *     Create a license contract, create a software 
   *       license and add the software license to the license pool
   * @param licensePoolId The name of the pool the license should be assigned.
   * @param licenseKey The license key.
   * @param licenseTyp Typ of a licnese, either "OEM", "VOLUME" or "RETAIL" 
   * @param partner Name of the license partner (optional).
   * @param conclusionDate Date of conclusion of license contract (optional)
   * @param notificationDate Date of notification that license is running out soon (optional).
   * @param notes This is the place for some notes (optional)
   * @param softwareLicenseId Identificator of a license (optional).
   * @param maxInstallations The number of clients use this license (optional).
   * @param boundToHost The name of the client the license is bound to (optional).
   * @param expirationDate The date when the license is running down (optional).
   */
  function createLicense($poolId, $licenseKey,$licenseTyp = "",  
        $partner = "",
        $conclusionDate = "",
        $notificationDate ="",
        $notes = "", 
        $softwareLicenseId = "",
        $maxInstallations = "",
        $boudToHost = "",
        $expirationDate = "")
  {

    $data= array();
    $data['licensePoolId']    = htmlentities($poolId);
    $data['licenseKey']        = htmlentities($licenseKey);

    // Append optional attributes 
    foreach(array("partner","conclusionDate","notificationDate","notes","softwareLicenseId",
          "licenseTyp","maxInstallations","boudToHost","expirationDate") as $attr){
      if(!empty($$attr)){
        $data[$attr] = $$attr;
      }
    }

    $res = $this->send_data("gosa_opsi_createLicense",$this->target,$data,TRUE);
    if(isset($res['XML'][0]['ANSWER_OPSI_GETSOFTWARELICENSE_HASH'])){
      return(TRUE);
    }
    return(FALSE);
  }


  /* @brief     Returns expirationDate, boundToHost, maxInstallation, licenseTyp, 
   *             licensePoolIds and licenseKeys for a given softwareLicense Id.
   * @param softwareLicenseId Identificator of a license.
   */
  function getLicense($softwareLicenseId)
  {
    $data= array();
    $data['softwareLicenseId'] = htmlentities($softwareLicenseId);
    $res = $this->send_data("gosa_opsi_getSoftwareLicense_hash",$this->target,$data,TRUE);
   
    if(isset($res['XML'][0]['ANSWER_OPSI_GETSOFTWARELICENSE_HASH'])){
      return($res);
    }
    return(FALSE);
  }


  /* 
   * @brief Returns softwareLicenseId, notes, licenseKey, hostId and licensePoolId for optional given licensePoolId and hostId
   * @param hostid Something like client_1.intranet.mydomain.de (optional).
   * @param licensePoolId The name of the pool (optional).
   */
  function getLicenseUsage($hostId = "", $licensePoolId = "")
  {
    $data= array();
    if(!empty($hostId)){
      $data['hostId'] = htmlentities($hostId);
    }
    if(!empty($hostId)){
      $data['licensePoolId'] = htmlentities($licensePoolId);
    }

    $res = $this->send_data("gosa_opsi_getSoftwareLicenseUsages_listOfHashes",$this->target,$data,TRUE);
    if(isset($res['XML'][0]['ANSWER_OPSI_GETSOFTWARELICENSEUSAGES_LISTOFHASHES'])){
      return($res['XML'][0]['RESULT']);
    }
    return(FALSE);
  }


  /* @brief   Assigns a software license to a host
   * @param   hostId Something like client_1.intranet.mydomain.de
   * @param   licensePoolId The name of the pool.
   */
  function addLicenseToHost($licensePoolId,$hostId)
  {
    $data= array();
    $data['licensePoolId'] = htmlentities($licensePoolId);
    $data['hostId'] = htmlentities($hostId);
    $res = $this->send_data("gosa_opsi_assignSoftwareLicenseToHost",$this->target,$data,TRUE);
    if(isset($res['XML'][0]['ANSWER_OPSI_ASSIGNSOFTWARELICENSETOHOST'])){
      return(TRUE);
    }
    return(FALSE);
  }


  /* 
   * @brief   Unassign a software license from a host.
   * @param   hostid Something like client_1.intranet.mydomain.de
   * @param   licensePoolId The name of the pool.
   */
  function removeLicenseFromHost($licensePoolId,$hostId)
  {
    $data= array();
    $data['licensePoolId'] = htmlentities($licensePoolId);
    $data['hostId'] = htmlentities($hostid);
    $res = $this->send_data("gosa_opsi_unassignSoftwareLicenseFromHost",$this->target,$data,TRUE);
    if(isset($res['XML'][0]['ANSWER_OPSI_UNASSIGNSOFTWARELICENSEFROMHOST'])){
      return(TRUE);
    }
    return(FALSE);
  }


  /* 
   * @brief Unassign all software licenses from a host
   * @param hostid Something like client_1.intranet.mydomain.de
   */
  function removeAllLicensesFromHost($hostId)
  {
    $data= array();
    $data['hostId'] = htmlentities($hostid);
    $res = $this->send_data("gosa_opsi_unassignAllSoftwareLicensesFromHost",$this->target,$data,TRUE);
    if(isset($res['XML'][0]['ANSWER_OPSI_UNASSIGNALLSOFTWARELICENSESFROMHOST'])){
      return(TRUE);
    }
    return(FALSE);
  }


  /* @brief 
   *   Returns the assigned licensePoolId and licenses, 
   *    how often the product is installed and at which host
   *    and the number of max and remaining installations for a given OPSI product.
   * @param productId Identificator of an OPSI product.
   */
  function getLicensesForProduct($productId)
  {
    $data= array();
    $data['productId'] = htmlentities($productId);
    $res = $this->send_data("gosa_opsi_getLicenseInformationForProduct",$this->target,$data,TRUE);
    if(isset($res['XML'][0]['ANSWER_OPSI_UNASSIGNALLSOFTWARELICENSESFROMHOST'])){
      return(TRUE);
    }
    return(FALSE);
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>