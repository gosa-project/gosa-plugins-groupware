<?php

 
/*! \brief Data Access Object for groupware
 */
class GroupwareDao{

    private $availableMethods;
    private $availableProperties;
    private $accountLocations;

    private $groupwarePluginRef = False;

    private static $gwFeatures  = array(
            "primaryMail" => array(	"get"=>"gwAcctGetPrimaryMailAddress", 
                "save"=>"gwAcctSetPrimaryMailAddress"),
            "mailLocations" => array("get"=>"gwGetMailboxLocations"),
            "quotaSize"  =>array(  "get"=>"gwAcctGetQuota", 
                "save"=>"gwAcctSetQuota",
                "delete"=>"gwAcctSetQuota"),
            "mailFilter"  =>array(  "get"=>"gwAcctGetFilters", 
                "save"=>"gwAcctSetFilters",
                "delete"=>"gwAcctDelFilter"),
            "alternateAddresses"  =>array(  "get"=>"gwAcctGetAlternateMailAddresses", 
                "save"=>"gwAcctSetAlternateMailAddresses",
                "delete"=>"gwAcctDelAlternateMailAddress"),
            "forwardingAddresses"  =>array( "get"=>"gwAcctGetMailForwardAddresses", 
                "save"=>"gwAcctSetMailForwardAddresses",
                "delete"=>"gwAcctDelMailForwardAddress"),
            "vacationMessage"  =>array(  	"get"=>"gwAcctGetOutOfOfficeReply", 
                "save"=>"gwAcctSetOutOfOfficeReply",
                "delete"=>"gwAcctDelOutOfOfficeReply"),
            "mailBoxWarnLimit"  =>array(  	"get"=>"", 
                "save"=>"",
                "delete"=>""),
            "mailBoxSendSizelimit"  =>array(  "get"=>"gwAcctGetQuota", 
                    "save"=>"gwAcctSetQuota",
                    "delete"=>"gwAcctDelQuota"),
            "mailBoxHardSizelimit"  =>array(  "get"=>"gwAcctGetMailLimit", 
                    "save"=>"gwAcctSetMailLimit",
                    "delete"=>"gwAcctDelMailLimit"),
            "mailBoxAutomaticRemoval"  =>array(  "get"=>"", 
                    "save"=>"",
                    "delete"=>""),
            "localDeliveryOnly"  =>array(  "get"=>"", 
                    "save"=>"",
                    "delete"=>""),
            "dropOwnMails"  =>array(  "get"=>"",
                    "save"=>"",
                    "delete"=>""),
            "accountProperties" => array("get"=>"gwAcctGetProperties",
                    "save"=>"gwAcctSetProperties",
                    "delete"=>"gwAcctDelProperties"),
			//mailFolder is a composite feature - 
			"mailFolder" => array( "get"=>"gwFolderList",
					"save"=>"gwFolderAdd",
                    "delete"=>"gwFolderDel"),
			"private_folderACLS" => array("get"=>"gwFolderGetMembers",
                    "save"=>"gwFolderSetMembers",
                    "delete"=>"gwFolderDelMember"),
			"mailLimit" => array("get"=>"gwAcctGetMailLimit",
                    "save"=>"gwAcctSetMailLimit")
                );


    /*! \brief Constructor sets the connection to the rpc service 
     * 		initializes the class
     */
    function __construct(&$pluginRef)
    {
        $this->groupwarePluginRef = &$pluginRef;

        $this->init();
    }


    /*! \brief Gets the capabilities of the server
     *          builds an array with availbale features and knows how to call get, save, delete functions of 
     *          groupware rpc. 
     */
    public function init()
    {
        $this->availableMethods = $this->groupwarePluginRef->rpcExec('gwGetCapabilities');
    }


    /*! \brief Generic saving method for all features defined in $gwFeatures
     *          which are the available features.
     */
    public function save($feature, $uid, $valueArray)
    {
        $function = GroupwareDao::$gwFeatures[$feature]["save"];
        if(is_array($valueArray)){
            $valueArray = array_merge(array($function, $uid), $valueArray);
        }else{
            $valueArray = array($function, $uid, $valueArray);
        }
        
        $result =  call_user_func_array(array($this->groupwarePluginRef, 'rpcExec'), $valueArray);
        return $result;
    }


    /*! \brief generic get method for all features defined in $gwFeatures
     * 		which are the available features.
     */
    public function get($feature, $valueArray)
    {		
        $function = GroupwareDao::$gwFeatures[$feature]["get"];
        if(is_array($valueArray)){
            $valueArray = array_merge(array($function), $valueArray);
        }else{
            $valueArray = array($function, $valueArray);
        }

        $result =  call_user_func_array(array($this->groupwarePluginRef, 'rpcExec'), $valueArray);
        return $result;
    }


    /*! \brief generic delete function for all features defined in $gwFeatures
     * 		which are the available features.
     * 
     * NOT YET IMPLEMENTED
     */
    public function del($feature, $valueArray)
    {
        
        echo "deletion of values is not implemented yet";
        /*
           $function = GroupwareDao::$gwFeatures[$feature]["delete"];
           $valueArray = array_merge(array($function), $valueArray);
           $result =  call_user_func_array(array($this->groupwarePluginRef, 'rpcExec'), $valueArray);
           return $result;
         */
    }


    /*! \brief determine availability of features.
     * 
     * @param methodName
     * @return boolean
     */
    public function gwFeatureAvailable($methodName)
    {
        return(isset($this->availableMethods[$methodName]) && ($this->availableMethods[$methodName]));
    }


    /*! \brief comprehensive user info will dessolve into one groupware call later
     * 		right now it will get all data of the features that are available in the plugin.
     * @param int $uid
     * @return assoc array "feature" => "gwValue";   
     */
    public function getComprehensiverUser( $uid)
    {
        //$hi = $this->groupwarePluginRef->rpcExec('gwGetCapabilities');
        $resultArr = array();

        if($this->groupwarePluginRef->isFeatureEnabled("primaryMail")){
            $resultArr["primaryMail"] = $this->get("primaryMail", array($uid));
        }

        if($this->groupwarePluginRef->isFeatureEnabled("alternateAddresses")){
            $resultArr["alternateAddresses"] = $this->get("alternateAddresses", array($uid));
        }

        if($this->groupwarePluginRef->isFeatureEnabled("forwardingAddresses")){
    		$FWaddresses = $this->get("forwardingAddresses", array($uid));		
            if($FWaddresses) {
                $resultArr["forwardingAddresses"] = array(key($FWaddresses));
            }
        }

        // Location dropdownlist - only readable
        if($this->groupwarePluginRef->isFeatureEnabled("mailLocations")){
            $resultArr["mailLocations"] = $this->groupwarePluginRef->rpcExec('gwGetMailboxLocations');
            $resultArr["mailLocation"] = $this->groupwarePluginRef->rpcExec('gwAcctGetLocation',$uid);
        }

		//getThefolderList from the user
		if($this->groupwarePluginRef->isFeatureEnabled("mailFolder")){
		
			//Folder Id from the User ...
			//user/$id/
			$resultArr["mailFolder"] = $this->get("mailFolder", array("user/".$uid."/"));
			
			foreach($resultArr["mailFolder"] as $val){
				$s = strpos($val, "user/");
				if($s == 0){
				
					$resultArr["mailFolder"][$val]["name"] = substr($val, strpos($val, "/", 5));
					
					$curAcl = $this->get("private_folderACLS", array("user/wiwu/Posteingang"));
					if(is_array($curAcl)){
						foreach($curAcl as $aclkey => $aclval){
							
							if (empty($aclkey)){
								$resultArr["mailFolder"][$val]["acls"][] = array( 
	 	                                "name" => $aclkey, 
	 	                                "acl"  => aclval,  
	 	                                "type"=>"user");
							}
							else{
								$resultArr["mailFolder"][$val]["acls"][] = array( 
	 	                                "name" => $aclkey, 
	 	                                "acl"  => GROUPWARE_RIGHTS_NONE,  
	 	                                "type"=>"user");
							}
						}
					}
				}
				else{
					// this case should never happen - unsetting the array anyway.
					unset($resultArr["mailFolder"][$val]);
				}
			}
			
			/*
			"RIGHTS_NONE", 0);
			"RIGHTS_READ",  LOOKUP | READ | STATUS);
			"RIGHTS_POST", RIGHTS_READ | POST);
			"RIGHTS_APPEND", RIGHTS_POST | INSERT);
			"RIGHTS_WRITE", RIGHTS_APPEND | WRITE | CREATE | DELETE);
            ""
             */
			
			//need a status for the array - setting them now for the example data.
            foreach($resultArr["mailFolder"] as $id => $folder){
                $resultArr["mailFolder"][$id]['status'] = '';
            }

        }

        //TODO: getLocation muss ebenfalls geholt werden 
        // Quota quotaUsage, quotaSize
        if($this->groupwarePluginRef->isFeatureEnabled("quotaSize") || 
                $this->groupwarePluginRef->isFeatureEnabled("quotaUsage") || 
                $this->groupwarePluginRef->isFeatureEnabled("mailBoxWarnLimit") || 
                $this->groupwarePluginRef->isFeatureEnabled("mailBoxSendSizelimit") || 
                $this->groupwarePluginRef->isFeatureEnabled("mailBoxHardSizelimit")){
            
            // example Array for Quota
            $quota = $this->get("quotaSize", array($uid));
            
            $resultArr["quotaSize"] = $quota["usage"];
            $resultArr["quotaSize"] = $quota["hold"];
            if($quota["warn_limit"] > 0){
                $resultArr["mailBoxWarnLimitEnabled"] = true;
                $resultArr["mailBoxWarnLimitValue"] = $quota["warn_limit"];
            }else{
                $resultArr["mailBoxWarnLimitEnabled"] = false;
                $resultArr["mailBoxWarnLimitValue"] = 0;
            }
            if($quota["send_limit"] > 0){
                $resultArr["mailBoxSendSizelimitEnabled"] = True;
                $resultArr["mailBoxSendSizelimitValue"] = $quota["send_limit"];
            }else{
                $resultArr["mailBoxSendSizelimitEnabled"] = FALSE;
                $resultArr["mailBoxSendSizelimitValue"] = 0; 	
            }
            if($quota["hard_limit"] > 0){
                $resultArr["mailBoxHardSizelimitEnabled"] = True;
                $resultArr["mailBoxHardSizelimitValue"] = $quota["hard_limit"];	
            }else{
                $resultArr["mailBoxHardSizelimitEnabled"] = FALSE;
                $resultArr["mailBoxHardSizelimitValue"] = 0;
            }
        }
		//get the current MailLimits ... if this feature is enabled.
		if($this->groupwarePluginRef->isFeatureEnabled("mailLimit")){
			$mailLimits = $this->get("mailLimit", array($uid));
			
			if($mailLimits["receive"] > 0){
                $resultArr["mailLimitReceiveEnabled"] = true;
                $resultArr["mailLimitReceiveValue"] = $mailLimits["receive"];
            }else{
                $resultArr["mailLimitReceiveEnabled"] = false;
                $resultArr["mailLimitReceiveValue"] = 0;
            }
            if($mailLimits["send"] > 0){
                $resultArr["mailLimitSendEnabled"] = true;
                $resultArr["mailLimitSendValue"] = $mailLimits["send"];
            }else{
                $resultArr["mailLimitSendEnabled"] = false;
                $resultArr["mailLimitSendValue"] = 0; 	
            }
		}		
        //this function seems to be broken on the server.
        //addding dummy

        if($this->groupwarePluginRef->isFeatureEnabled("vacationMessage")){
			$vacArray = $this->get("vacationMessage", array($uid));
			
            $resultArr["vacationMessage"] = $vacArray["message"];
			if(!empty($resultArr["vacationMessage"])){
				$resultArr["vacationEnabled"] = true;
			}
            //$resultArr["vacationMessage"] = "dummy Vacation message - (getOutOfOfficeReply currently throws errors )";
        }   
        return $resultArr;
    }
}
?>
