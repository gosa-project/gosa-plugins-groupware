<?php
/*
   This code is part of GOsa (https://gosa.gonicus.de)
   Copyright (C) 2003  Cajus Pollmeier

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


/* Preset display */
$display= "";

if (!$remove_lock){
  /* Reset requested? */
  if (isset($_POST['edit_cancel']) || isset($_POST['password_cancel']) || 
      (isset($_GET['reset']) && $_GET['reset'] == 1)){
    del_lock ($ui->dn);
    sess_del ('edit');
    sess_del ('user');
  }

  /* Create user object on demand */
  if (!isset($_SESSION['user']) || (isset($_GET['reset']) && $_GET['reset'] == 1)){
    $_SESSION['user']= new user ($config, $ui->dn);
  }
  $user= $_SESSION['user'];

  /* save changes back to object */
  if (isset ($_SESSION['edit'])){
    $user->save_object ();
  }

  /* Enter edit mode? */
  if ((isset($_POST['edit'])) && (!isset( $_SESSION['edit']))){

    /* Check locking */
    if (($username= get_lock($ui->dn)) != ""){
      $_SESSION['back_plugin']= $plug;
      gen_locked_message ($username, $ui->dn);
      exit ();
    }

    /* Lock the current entry */
    add_lock ($ui->dn, $ui->dn);
    $_SESSION['dn']= $ui->dn;
    $_SESSION['edit']= TRUE;
  }

  /* Adjust acl's to mode */
  if (isset($_SESSION['edit'])){
    $acl= get_permissions ($ui->dn, $ui->subtreeACL);
    $user->acl= get_module_permission($acl, "user", $ui->dn);
  } else {
    $acl= get_permissions ($ui->dn, $ui->subtreeACL);
    $editacl= get_module_permission($acl, "user", $ui->dn);
    $user->acl= "#none#";
  }

  /* Perform password change */
  if (isset($_POST['password_finish'])){

    /* For security reasons, check if user is allowed to set password again */
    if (chkacl($user->acl, "password") == "" || chkacl($user->acl, "create")){

      /* Check input and feed errors into 'message' */
      $message= array();

      /* Sanity checks... */
      if ($_POST['new_password'] != $_POST['repeated_password']){

        /* Matching passwords in new and repeated? */
        $message[]= _("The passwords you've entered as 'New password' and 'Repeated new password' do not match.");
      } else {

        /* Empty password is not permitted by default. */
        if ($_POST['new_password'] == ""){
          $message[]= _("The password you've entered as 'New password' is empty.");
        }
      }

      /* Errors, or change password? */
      if (count($message) != 0){

        /* Show error message and continue editing */
        show_errors ($message);
        $display.= $smarty->fetch(get_template_path('password.tpl', TRUE));
        return ($display);
      }
      change_password ($user->dn, $_POST['new_password'], 0, $user->pw_storage);
      gosa_log ("Password for '".$user->dn."' has been changed");

    } else {

      /* Missing permissions, show message */
      print_red (_("You are not allowed to set your password!"));
    }

    $user->acl= "#none#";
    del_lock ($ui->dn);
    sess_del ('user');
  }

  /* save changes to LDAP and disable edit mode */
  if (isset($_POST['edit_finish'])){

    /* Perform checks */
    $message= $user->check ();

    /* No errors, save object */
    if (count ($message) == 0){
      $user->save ();
      gosa_log ("User/generic object'".$ui->dn."' has been saved");
      $user->acl= "#none#";
      $editacl= get_module_permission($acl, "user", $ui->dn);
      del_lock ($ui->dn);
      sess_del ('edit');

      /* Need to reset the password? */
      if($user->password_change_needed()){
        /* Save object */
        $_SESSION['user']= $user;

        $display.= $smarty->fetch(get_template_path('password.tpl', TRUE));
        return ($display);
      } else {
	sess_del ('user');
      }

    } else {
      /* Errors found, show message */
      show_errors ($message);
    }
  }

  /* Execute formular */
  $display.= $user->execute ();

  /* Store changes  in session */
  if (isset ($_SESSION['edit'])){
    $_SESSION['user']= $user;
  }

  $info = "";
  /* Show page footer depending on the mode */
  if (!$user->cert_dialog && !$user->picture_dialog && $user->is_account){
    $display.= "<p class=\"plugbottom\">";

    /* Are we in edit mode? */
    if (isset($_SESSION['edit'])){
      $display.= "<input type=submit name=\"edit_finish\" value=\""._("Save")."\">\n";
      $display.= "&nbsp;";
      $display.= "<input type=submit name=\"edit_cancel\" value=\""._("Cancel")."\">\n";

      $info= "<img class=\"center\" alt=\"\" align=\"middle\" src=\"".get_template_path('images/closedlock.png').
             "\"> ".$ui->dn."&nbsp;";
    } else {
      /* 'viewid' may be set by i.e. the phonelist plugin. This
         means that we want to go back... */
      if (isset ($viewid)){
        $str= _("Back");
        $fn = "back";
      } else {
        $str= _("Edit");
        $fn = "edit";
        $info= "<img class=\"center\" alt=\"\" align=\"middle\" src=\"".get_template_path('images/openlock.png').
               "\"> ".$ui->dn."&nbsp;";
      }

      if ($fn == "edit" && isset($editacl) && $editacl != "#none#"){
        $info.= "<img class=\"center\" alt=\"\" align=\"middle\" src=\"".get_template_path('images/lamp.png')."\"> ".
                _("Click the 'Edit' button below to change informations in this dialog");
        $display.= "<input type=submit name=\"$fn\" value=\"$str\">\n";
      }
      $display.= "<input type=\"hidden\" name=\"ignore\">\n";
    }
    $display.= "</p>\n";
  }

  /* Page header*/
  $display= print_header(get_template_path('images/personal.png'),
                         _("Generic user information"), $info).$display;

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>