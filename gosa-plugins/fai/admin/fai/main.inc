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


/* Remove locks created by this plugin
*/
if ($remove_lock){
  if(session::is_set('FAI')){
      $macl = session::get('FAI');
      $macl->remove_lock();
  }
}

/* Remove this plugin from session
*/
if ( $cleanup ){
  session::un_set('FAI');
}else{

	/* Create usermanagement object on demand */
	if (!session::is_set('FAI')){
		$FAI= new faiManagement($config, $ui);
		$FAI->set_acl_category("fai");
		session::set('FAI',$FAI);
	}
	$FAI = session::get('FAI');
	$FAI->save_object();
	$output= $FAI->execute();

	/* Reset requested? */
	if (isset($_GET['reset']) && $_GET['reset'] == 1){
		session::un_set ('FAI');
	}
	/* Page header*/
	if (get_object_info() != ""){
		$display= print_header(get_template_path($FAI->plIcon), _("Software deployment"), "<img alt=\"\" align=\"middle\" src=\"".get_template_path('images/lists/locked.png')."\"> ".LDAP::fix(get_object_info()));
	} else {
		$display= print_header(get_template_path($FAI->plIcon), _("Software deployment"));
	}
	$display.= $output;

	session::set('FAI',$FAI);
}

?>