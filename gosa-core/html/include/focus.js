var browserType = "Not dectected, adjust focus.js";
var firefoxType;

if (document.layers) 
{
		browserType = "nn4";
}

if (document.all) 
{
		browserType = "ie";
}

if (window.navigator.userAgent.toLowerCase().match("gecko")) 
{
		browserType= "gecko";
}

if (window.navigator.userAgent.toLowerCase().match(/opera/i)) 
{
		browserType= "gecko";
}

if (browserType=="gecko" && window.navigator.userAgent.toLowerCase().match("firefox\/2")) 
{
		firefoxType= "firefox2";
}else{
		firefoxType= "firefox";
}

netscape = "";
ver= navigator.appVersion; len = ver.length;

for (iln = 0; iln < len; iln++){
  if (ver.charAt(iln) == "("){
    break;
  }
}
netscape= (ver.charAt(iln+1).toUpperCase() != "C");


/* Toggle checkbox that matches regex */
function chk_set_all(regex,value)
{
        for (var i = 0; i < document.mainform.elements.length; i++) {
                var _id=document.mainform.elements[i].id;
                if(_id.match(regex)) {
                        document.getElementById(_id).checked= value;
                }
        }
}


function toggle_all_(regex,state_object)
{
		state = document.getElementById(state_object).checked;
		chk_set_all(regex, state);
}


function scrollDown() {
  document.body.scrollTop = document.body.scrollHeight - document.body.clientHeight;
  timeout= setTimeout("scrollDown()", 500);
}

/* Scroll down the body frame */
function scrollDown2()
{
    document.body.scrollTop = document.body.scrollHeight - document.body.clientHeight;
}


/* Toggle checkbox that matches regex */
function acl_set_all(regex,value)
{
				for (var i = 0; i < document.mainform.elements.length; i++) {
								var _id=document.mainform.elements[i].id;
								if(_id.match(regex)) {
												document.getElementById(_id).checked= value;
								}
				}
}

/* Toggle checkbox that matches regex */
function acl_toggle_all(regex)
{
				for (var i = 0; i < document.mainform.elements.length; i++) {
								var _id=document.mainform.elements[i].id;
								if(_id.match(regex)) {
												if (document.getElementById(_id).checked == true){
																document.getElementById(_id).checked= false;
												} else {
																document.getElementById(_id).checked= true;
												}
								}
				}
}


var enable_keyPress = true;
function keyPress(DnEvents) {

	/* We are forced to skip this Keyboard input filtering 
   *  (enable_keyPress was set to false in the HTML content)
   */
	if(!enable_keyPress) return;

  // determines whether Netscape or Internet Explorer
  k = (netscape) ? DnEvents.keyCode : window.event.keyCode;
  if (k == 13) { // enter key pressed
		if(typeof(nextfield)!='undefined') {
			if(nextfield == 'login') {
    	  return true; // submit, we finished all fields
    	} else { // we are not done yet, send focus to next box
      	eval('document.mainform.' + nextfield + '.focus()');
      	return false;
    	}
  	} else {
			if(netscape) {
				if(DnEvents.target.type == 'textarea') {
					return true;
				} else if (DnEvents.target.type != 'submit') {
					// TAB
					var thisfield = document.getElementById(DnEvents.target.id);
					for (i = 0; i < document.forms[0].elements.length; i++) {
						if(document.forms[0].elements[i].id==thisfield.id) {
							// Last form element on page?
							if(i!=document.forms[0].elements.length-1) {
								document.forms[0].elements[i+1].focus();
							}
						}
					}
					return false;
				} else {
					return true;
				}
			// Check for konqueror
			} else if(document.clientWidth) {
				// do nothing ATM
			} else {
				if(window.event.srcElement.type == 'textarea') {
					return true;
				} else if (window.event.srcElement.type != 'submit') {
					// TAB
					var thisfield = document.getElementById(window.event.srcElement.id);
					for (i = 0; i < document.forms[0].elements.length; i++) {
						if(document.forms[0].elements[i].id==thisfield.id) {
							// Last form element on page?
							if(i!=document.forms[0].elements.length-1) {
								document.forms[0].elements[i+1].focus();
							}
						}
					}
					return false;
				} else {
					return true;
				}
			}
		}
	} else if (k==9) {
		// Tab key pressed
		if(netscape) {
			if(DnEvents.target.type == 'textarea') {
				document.getElementById(DnEvents.target.id).value+="\t";
		 		return false;
			}
		// Check for konqueror
		} else if(document.clientWidth) {
			// do nothing ATM
		} else {
		 	if(window.event.srcElement.type == 'textarea') {
				document.getElementById(window.event.srcElement.id).value+="\t";
		 		return false;
		 	}
		}
	}
}

function changeState(myField) {
	if(document.getElementById(myField) != null){
	  document.getElementById(myField).disabled=(document.getElementById(myField).disabled)?false:true;
	}
}

function setHidden(str) {
	type = document.getElementById(str).style.display;
	if((type=='')||(type=='block')) {
		document.getElementById(str).style.display='none';
	}else{
		document.getElementById(str).style.display='block';
	}
}
function changeSelectState(triggerField, myField) {
  if (document.getElementById(triggerField).value != 2){
	  document.getElementById(myField).disabled= true;
  } else {
	  document.getElementById(myField).disabled= false;
  }
}

function changeSubselectState(triggerField, myField) {
  if (document.getElementById(triggerField).checked == true){
	  document.getElementById(myField).disabled= false;
  } else {
	  document.getElementById(myField).disabled= true;
  }
}

function changeTripleSelectState(firstTriggerField, secondTriggerField, myField) {
  if (
  	document.getElementById(firstTriggerField).checked == true &&
	document.getElementById(secondTriggerField).checked == true){
	  document.getElementById(myField).disabled= false;
  } else {
	  document.getElementById(myField).disabled= true;
  }
}

<!-- Second field must be non-checked -->
function changeTripleSelectState_2nd_neg(firstTriggerField, secondTriggerField, myField) {
  if (
  	document.getElementById(firstTriggerField).checked == true &&
	document.getElementById(secondTriggerField).checked == false){
	  document.getElementById(myField).disabled= false;
  } else {
	  document.getElementById(myField).disabled= true;
  }
}
// work together to analyze keystrokes
if (netscape){
  if(firefoxType== "firefox") {
		window.captureEvents(Event.KEYPRESS);
	}
	window.onkeypress= keyPress;
} else {
	document.onkeydown= keyPress;
}

function hide(element) {
  if (browserType == "gecko" )
     document.poppedLayer = document.getElementById(element);
  else if (browserType == "ie")
     document.poppedLayer = document.all[element];
  else
     document.poppedLayer = document.layers[element];
 	document.poppedLayer.style.visibility = "hidden";
}

function show(element) {
  if (browserType == "gecko" )
     document.poppedLayer = document.getElementById(element);
  else if (browserType == "ie")
     document.poppedLayer = document.all[element];
  else
     document.poppedLayer = document.layers[element];
  document.poppedLayer.style.visibility = "visible";
}

function GOsa_toggle(element) {
  if (browserType == "gecko" )
     document.poppedLayer = document.getElementById(element);
  else if (browserType == "ie")
     document.poppedLayer = document.all[element];
  else
     document.poppedLayer = document.layers[element];

  if (document.poppedLayer.style.visibility == "visible") {
	  hide (element);
	} else {
	  show (element);
	}
}

function popup(target, name) {
	var mypopup= 
		window.open(
			target,
			name,
			"width=600,height=700,location=no,toolbar=no,directories=no,menubar=no,status=no,scrollbars=yes"
		);
	mypopup.focus();
	return false;
}

function js_check(form) {
	form.javascript.value = 'true';
}

function divGOsa_toggle(element) {
	var cell;
	var cellname="tr_"+(element);

	if (browserType == "gecko" ) {
    document.poppedLayer = document.getElementById(element);
		cell= document.getElementById(cellname);

	  if (document.poppedLayer.style.visibility == "visible") {
		  hide (element);
			cell.style.height="0px";
			document.poppedLayer.style.height="0px";
		} else {
		  show (element);
			document.poppedLayer.style.height="";
			if(document.defaultView) {
				cell.style.height=document.defaultView.getComputedStyle(document.poppedLayer,"").getPropertyValue('height');
			}
		}
	} else if (browserType == "ie") {
    document.poppedLayer = document.getElementById(element);
		cell= document.getElementById(cellname);
	  if (document.poppedLayer.style.visibility == "visible") {
		  hide (element);
			cell.style.height="0px";
			document.poppedLayer.style.height="0px";
			document.poppedLayer.style.position="absolute";
		} else {
		  show (element);
			cell.style.height="";
			document.poppedLayer.style.height="";
			document.poppedLayer.style.position="relative";
		}
	}
}

function adjust (e) {
	adjust_height(e);
	adjust_width(e);
}

// Automatic resize (height) of divlists
function adjust_height(e) {
	if (!e) e=window.event;
	if (document.getElementById("menucell") && document.getElementById("d_scrollbody")) {
		var inner_height= window.innerHeight;
		var min_height= 450;
		var px_height= min_height;
		var suggested= px_height;
	
		// document.defaultView allows access to the rendered size of elements and should be supported by modern browsers
		if(document.defaultView) {
			var menu_height=parseInt(document.defaultView.getComputedStyle(document.getElementById("menucell"),"").getPropertyValue('height'));
	
			// Minimum height for divlist should be the bottom edge of the menu
			min_height= menu_height-197;
			suggested= min_height;
			if((inner_height-230)-suggested>0) {
				suggested= inner_height-230;
			}
		
		// IE uses other height specifications
		} else if (browserType == "ie") {
			suggested= document.all.menucell.offsetHeight;
			offset= absTop(d_scrollbody);
			suggested-= offset;
			if((inner_height-230)-suggested>0) {
				suggested= inner_height-230;
			}
		}

		/* Reduce height if a list footer is set */
		if(document.getElementById("t_scrollfoot")){
			suggested = suggested -20;
		}

		document.getElementById("d_scrollbody").style.height=suggested+"px";
	}
	return true;
}

function absTop(e) {
	return (e.offsetParent)?e.offsetTop+absTop(e.offsetParent) : e.offsetTop;
}

// Automatic resize (width) of divlists
function adjust_width(e) 
{
	
	/* Get event ... it seems to be unused here ...*/
	if (!e) {
		e=window.event;
	}

	// Known to not work with IE
	if(document.defaultView && document.getElementById("t_scrolltable")) {

		// Get current width of divlist 
		var div_width	=	parseInt(document.defaultView.getComputedStyle(document.getElementById("t_scrolltable"),"").getPropertyValue('width'));
	
		// Get window width
		var width= parseInt(window.innerWidth);

		// Resize the body cells, 470 represents the info box and the navigation part 
		var diff= width	-	div_width	-	470;
                if(document.getElementById('d_save')) {
                  diff= width - div_width - document.getElementById('d_save').value;
                }
		
		// window has been upscaled
		if(div_width+diff>=600) {
			document.getElementById('d_scrollbody').style.width=div_width+diff+"px";
			document.getElementById('t_scrollbody').style.width=(div_width-19)+diff+"px";
	
			// Resize the Header cells (only the relative-width ones)
			document.getElementById('t_scrollhead').style.width=div_width+diff+"px";

		// window has been downscaled, we must reset the div to 600px
		} else if (width < 930) {
			// Reset layout (set width to 600px)
			div_width=600;
			document.getElementById('d_scrollbody').style.width=div_width+"px";
			document.getElementById('t_scrollbody').style.width=(div_width-19)+"px";
	
			// Resize the Header cells (only the relative-width ones)
			document.getElementById('t_scrollhead').style.width=div_width+"px";
		}
	} else if(document.defaultView && document.getElementById("t_scrolltable_onlywidth")){
		// Resize the div
		var div_width=parseInt(document.defaultView.getComputedStyle(document.getElementById("t_scrolltable_onlywidth"),"").getPropertyValue('width'));
		var width= parseInt(window.innerWidth);

		// Resize the body cells
		var diff= width-div_width-200;
		
		// window has been upscaled
		if(div_width+diff>=600) {
			if(document.getElementById('d_scrollbody_onlywidth')){
				document.getElementById('d_scrollbody_onlywidth').style.width=div_width+diff+"px";
			}
			document.getElementById('t_scrollbody_onlywidth').style.width=(div_width-19)+diff+"px";
	
			// Resize the Header cells (only the relative-width ones)
			document.getElementById('t_scrollhead_onlywidth').style.width=div_width+diff+"px";

		// window has been downscaled, we must reset the div to 600px
		} else if (width < 930) {
			// Reset layout (set width to 600px)
			div_width=600;
			if(document.getElementById('d_scrollbody_onlywidth')){
				document.getElementById('d_scrollbody_onlywidth').style.width=div_width+"px";
			}
			document.getElementById('t_scrollbody_onlywidth').style.width=(div_width-19)+"px";
	
			// Resize the Header cells (only the relative-width ones)
			document.getElementById('t_scrollhead_onlywidth').style.width=div_width+"px";
		}
	} else {
		// IE

	}
}


/* Set focus to first valid input field
    avoid IExplorer warning about hidding or disabled fields
*/
function focus_field()
{
    var i     = 0;
    var e     = 0;
    var found = false;
    var element_name = "";
    var element =null;

    while(focus_field.arguments[i] && !found){

        var tmp = document.getElementsByName(focus_field.arguments[i]);
        for(e = 0 ; e < tmp.length ; e ++ ){

            if(tmp[e].disabled != true &&  tmp[e].type != "none" && tmp[e].type != "hidden" ){
                found = true;
                element = tmp[e];
            }
        }
        i++;
    }

    if(element && found){
        element.blur();
        element.focus();
    }
}


/*  This function pops up messages from message queue 
		All messages are hidden in html output (style='display:none;').
		This function makes single messages visible till there are no more dialogs queued.

		hidden inputs: 
			current_msg_dialogs		- Currently visible dialog
			closed_msg_dialogs		- IDs of already closed dialogs 
			pending_msg_dialogs		- Queued dialog IDs. 
*/
function next_msg_dialog()
{
		var s_pending = "";
		var a_pending = new Array();
		var i_id			= 0;
		var i					= 0;
		var tmp				= "";
		var ele 			= null;
		var ele2 			= null;
		var cur_id 		= "";

		if(document.getElementById('current_msg_dialogs')){
				cur_id = document.getElementById('current_msg_dialogs').value;
				if(cur_id != ""){
						ele = document.getElementById('e_layer' + cur_id);
						ele.onmousemove = "";
						hide('e_layer' + cur_id); 	
						document.getElementById('closed_msg_dialogs').value += "," + cur_id;
						document.getElementById('current_msg_dialogs').value= ""; 
				}
		}

		if(document.getElementById('pending_msg_dialogs')){
				s_pending = document.getElementById('pending_msg_dialogs').value;
				a_pending = s_pending.split(",");
				if(a_pending.length){
						i_id = a_pending.pop();
						for (i = 0 ; i < a_pending.length; ++i){
								tmp = tmp + a_pending[i] + ',';
						}
						tmp = tmp.replace(/,$/g,"");
						if(i_id != ""){
								ele = document.getElementById('e_layer' + i_id);
								ele3 = document.getElementById('e_layerTitle' + i_id);
								ele.style.display= 'block'	;
								document.getElementById('pending_msg_dialogs').value= tmp;
								document.getElementById('current_msg_dialogs').value= i_id;
								ele2 = document.getElementById('e_layer2') ;
								ele3.onmousedown = start_move_div_by_cursor;
								ele2.onmouseup 	= stop_move_div_by_cursor;
								ele2.onmousemove = move_div_by_cursor;
						}else{
								ele2 = document.getElementById('e_layer2') ;
								ele2.style.display ="none";
						}
				}
		}
}


/* Drag & drop for message dialogs */
var enable_move_div_by_cursor = false;		// Indicates wheter the div movement is enabled or not 
var mouse_x_on_div	= 0;									// 
var mouse_y_on_div 	= 0;
var div_offset_x  	= 0;
var div_offset_y  	= 0;

/* Activates msg_dialog drag & drop
 * This function is called when clicking on a displayed msg_dialog 
 */
function start_move_div_by_cursor(e)
{
		var x = 0; 
		var y = 0;	
		var cur_id = 0;
		var dialog = null;
		var event = null;

		/* Get current msg_dialog position
     */
		cur_id = document.getElementById('current_msg_dialogs').value;
		if(cur_id != ""){
				dialog = document.getElementById('e_layer' + cur_id);
				x = dialog.style.left;
				y = dialog.style.top;
				x = x.replace(/[^0-9]/g,"");
				y = y.replace(/[^0-9]/g,"");
				if(!y) y = 1;
				if(!x) x = 1;
		}

		/* Get mouse position within msg_dialog 
     */
		if(window.event){
				event = window.event;
				if(event.offsetX){
						div_offset_x   = event.clientX -x;
						div_offset_y   = event.clientY -y;
						enable_move_div_by_cursor = true;
				}
		}else if(e){
				event = e;
				if(event.layerX){
						div_offset_x	= event.screenX -x;
						div_offset_y	= event.screenY -y;
						enable_move_div_by_cursor = true;
				}
		}
}


/* Deactivate msg_dialog movement 
*/
function stop_move_div_by_cursor()
{
		mouse_x_on_div = 0;
		mouse_y_on_div = 0;
		div_offset_x = 0;
		div_offset_y = 0;
		enable_move_div_by_cursor = false;
}


/* Move msg_dialog with cursor */
function move_div_by_cursor(e)
{
		var event 				= false;
		var mouse_pos_x		= 0;
		var mouse_pos_y 	= 0;
		var	cur_div_x = 0;
		var cur_div_y = 0;
		var cur_id	= 0;
		var dialog = null;


		if(undefined !== enable_move_div_by_cursor && enable_move_div_by_cursor == true){

				if(document.getElementById('current_msg_dialogs')){

						/* Get mouse position on screen 
             */
						if(window.event){
								event = window.event;
								mouse_pos_x  =event.clientX;
								mouse_pos_y  =event.clientY;
						}else if (e){
								event = e;
								mouse_pos_x  =event.screenX;
								mouse_pos_y  =event.screenY;
						}else{
							return;
						}

						/* Get id of current msg_dialog */
						cur_id = document.getElementById('current_msg_dialogs').value;
						if(cur_id != ""){
								dialog = document.getElementById('e_layer' + cur_id);
	
								/* Calculate new position */
								cur_div_x = mouse_pos_x - div_offset_x;
								cur_div_y = mouse_pos_y - div_offset_y;

								/* Ensure that dialog can't be moved out of screen */
								if(cur_div_x < 0 ) cur_div_x = 0
								if(cur_div_y < 0 ) cur_div_y = 0
							
								/* Assign new values */
								dialog.style.left = (cur_div_x ) + "px";
								dialog.style.top  = (cur_div_y ) + "px";
						}
				}
		}
}

function send_menu_action(str)
{
		if(str != "" && str != "#"){
				if(document.getElementById('menu_action')){
						document.getElementById('menu_action').value=str;
						document.mainform.submit();
				}
	}
}

// vim:ts=2:syntax