<?php

  class dfsgeneric extends plugin {

    /* Needed values and lists */
    var $base             = "";
    var $cn               = "";

    /* attribute list for save action */
    var $attributes     = array();
    var $objectclasses  = array();

    function dfsgeneric(&$config, $dn) {
      plugin::plugin($config, $dn);
      $this->dn = $dn;
      $this->orig_dn = $dn;
    }
 
    function execute() {
      /* Call parent execute */
      plugin::execute();
      
      $smarty= get_smarty();

      if (($this->dn == "new") || ($this->dn == "")) {
        $smarty->assign("sambasharename", "");
        $smarty->assign("sharedescription", "");
        $smarty->assign("fileserver", "");
        $smarty->assign("fileservershare", "");
        $smarty->assign("location", "");
        $smarty->assign("dfsdescription", "");
      } else {
        $ldap = $this->config->get_ldap_link();

        $base = get_base_from_people($this->dn);
        $ou = get_ou("DFS");

        $tmp = preg_split('/\//', $this->dn, 2);
        $this->loc = $tmp[0];
        $this->sambasharename = $tmp[1];

        $ldap->cd("$ou $base");
        $ldap->search(("ou=$this->loc"), array("description"));
        $dfs_desc = $ldap->fetch();
        $this->dfsdescription = $dfs_desc['description'][0];

        $ldap->cd("ou=$this->loc, $ou $base");
        $ldap->search("(&(sambaShareName=$this->sambasharename)(objectclass=sambaShare))", array("description", "documentLocation"));

        $details = $ldap->fetch();
        $this->sharedescription = $details['description'][0];
        $tmp = preg_split('/\\\\/', $details['documentLocation'][0], 2);

        $this->fileserver = preg_replace("/msdfs:/", "", $tmp[0]);
        $this->share = preg_replace("/\\\/", "", $tmp[1]);
      
        /* Fill array */
        $smarty->assign("sambasharename", $this->sambasharename);
        $smarty->assign("sharedescription", $this->sharedescription);
        $smarty->assign("fileserver", $this->fileserver);
        $smarty->assign("fileservershare", $this->share);
        $smarty->assign("location", $this->loc);
        $smarty->assign("dfsdescription", $this->dfsdescription);
      }

      /* Show main page */
      return ($smarty->fetch (get_template_path('generic.tpl', TRUE)));
    }

    function check() 
	{
      plugin::check(); 
      $message = array();

      $ldap = $this->config->get_ldap_link();
      $base = get_base_from_people($this->ui->dn);
      $ou = get_ou("DFS");
      $dn_explode = explode("/", $this->dn);
      $sub_ou = $dn_explode[0];
      $sambaShareName = $dn_explode[1] . "/" . $dn_explode[2] . "/" . $dn_explode[3];
      $dn = "sambaShareName=$sambaShareName,ou=$sub_ou,$ou$base";
      $ldap->cat($dn);
      $attrs = $ldap->fetch();
      
      if ($this->orig_dn == "new" && !($attrs == FALSE)) {
		$message[] = msgPool::duplicated("Dfs Share");
      } elseif ($this->orig_dn != $this->dn && !($attrs == FALSE)) {
		$message[] = msgPool::duplicated("Dfs Share");
      }
      
      if ($this->dn == "new" || $this->dn == "") {
        $this->sambasharename = $_POST['sambaShareName'];
        $this->sharedescription = $_POST['description'];
        $this->fileserver = $_POST['fileserver'];
        $this->share = $_POST['fileservershare'];
        $this->loc = $_POST['location'];
      }
      
      # required fields set?
      if ($this->sambasharename == "") {
		$message[] = msgPool::required("Name");
      }
      if ($this->sharedescription == "") {
		$message[] = msgPool::required("Description");
      }
      if ($this->fileserver == "") {
		$message[] = msgPool::required("Fileserver");
      }
      if ($this->share == "") {
		$message[] = msgPool::required("Share");
      }
      if ($this->loc == "") {
		$message[] = msgPool::required("Location");
      }

      return $message;
    }
    
    function save() {
      
      plugin::save();

      # get the current values
      $this->sambasharename = $_POST['sambaShareName'];
      $this->sharedescription = $_POST['description'];
      $this->fileserver = $_POST['fileserver'];
      $this->share = $_POST['fileservershare'];
      $this->loc = $_POST['location'];
          
      # set the attribs
      $this->attrs["sambaShareName"]   = "$this->sambasharename,ou=$this->loc,$ou$base";
      $this->attrs["objectClass"][]    = "top";
      $this->attrs["objectClass"][]    = "SambaShare";
      $this->attrs["objectClass"][]    = "extensibleObject"; 
      $this->attrs["sambaShareName"]   = $this->sambasharename;
      $this->attrs["description"]      = $this->sharedescription;
      $this->attrs["documentLocation"] = "msdfs:$this->fileserver\\\\$this->share";

      $ldap = $this->config->get_ldap_link();

      if ($this->dn == "new" || $this->dn == "") {
        $base = get_base_from_people($this->ui->dn);
        $ou = get_ou("DFS");
        $this->basedn = "sambaShareName=$this->sambasharename,ou=$this->loc,$ou$base";
        $ldap->cd($this->basedn);
        $ldap->add($this->attrs);
      } else {
        # try to find entry
        $base = get_base_from_people($this->ui->dn);
        $ou = get_ou("DFS");
        #$dn_explode = explode("/", $this->sambasharename);
        #$sub_ou = $dn_explode[0];
        #$sambaShareName = $dn_explode[1] . "/" . $dn_explode[2] . "/" . $dn_explode[3];
        $dn = "sambaShareName=$this->sambasharename,ou=$this->loc,$ou$base";
        $ldap->cat($dn);
        $attrs = $ldap->fetch();
          
        $this->basedn = "sambaShareName=$this->sambasharename,ou=$this->loc,$ou$base";
        $ldap->cd($this->basedn);
        $nr = count($attrs);
        
        if (count($attrs)) {
          # modify if found
          $ldap->modify($this->attrs);
        } else {
          # add
          $ldap->add($this->attrs);
        }

	if (!$ldap->success()){
		msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
	}
      }
    }

	function save_object() {
		plugin::save_object();
		$this->base = $_POST['base'];
	}

    function delete() {
    
      plugin::delete();

      # get the current values
      $this->sambasharename = $_POST['sambaShareName'];
      $this->sharedescription = $_POST['description'];
      $this->fileserver = $_POST['fileserver'];
      $this->share = $_POST['fileservershare'];
      $this->loc = $_POST['location'];
          
      $base = get_base_from_people($this->ui->dn);
      $ou = get_ou("DFS");
      $this->basedn = "sambaShareName=$this->sambasharename,ou=$this->loc,$ou$base";

      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->basedn);
      $ldap->rmdir($this->basedn);
      if (!$ldap->success()){
	      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->basedn, LDAP_DEL, get_class()));
      }
    }
      
  }
