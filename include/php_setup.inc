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

function gosaRaiseError($errno, $errstr, $errfile, $errline)
{
  global $error_collector,$error_collector_mailto,$config;

  if(function_exists("gosa_log")){
    gosa_log($errno." ".$errstr." ".$errfile." ".$errline);
  }

  /* Return if error reporting is set to zero */
  if (error_reporting() == 0){
    return;
  }

  /* Workaround for buggy imap_open error outputs */
  if (preg_match('/imap_open/', $errstr)){
    return;
  }

  /* FIXME: Workaround for PHP5 error message flooding. The new OOM
     code want's us to use public/protected/private instead of flat
     var declarations. For now I can't workaround this - let's ignore
     the messages till the next major release which may drop support
     for PHP4. */
  if (preg_match('/var: Deprecated./', $errstr)){
    return;
  }

  /* FIXME: Same as above. Compatibility does error flooding.*/
  if (preg_match('/zend.ze1_compatibility_mode/', $errstr)){
    return;
  }

  /* Hide ldap size limit messages */
  if (preg_match('/ldap_error/', $errstr)){
    if (preg_match('/sizelimit/', $errstr)){
      return;
    }
  }

  if((isset($config->data))){ 
    if((isset($config->data['MAIN']['DISPLAYERRORS']))&&(!preg_match("/^true$/i",$config->data['MAIN']['DISPLAYERRORS']))){

      /* Write to syslog */
      gosa_log ("PHP error: $errstr ($errfile, line $errline)");
      return;
    }
  }


  /* Create header as needed */
  if ($error_collector == ""){

    /* Mailto body header */ 
    if(function_exists("prepare4mailbody")){
    $error_collector_mailto .=prepare4mailbody(
                                "Oups. Seems like you've catched some kind of bug inside GOsa/PHP. You may want to help ".
                                "us to improve the software stability. If so, please provide some more information below.".
                                "\n\n".
                                "*** GOsa bug report ***".
                                "\nGOsa Version: ".get_gosa_version().
                                "\nDate: ".date("d.m.Y").
                                "\nTime: ".date("H:i:s").
                                "\nUser-Agent: ".$_SERVER['HTTP_USER_AGENT']." ".
                                "(Javascript is ".( (isset($_SESSION['js']) && $_SESSION['js']==FALSE) ? "inactive" : "active" ).")".
                                "\n\n".
                                "Please describe what you did to produce this error as detailed as possible. Can you ".
                                "reproduce this bug using the demo on http://www.gosa-project.org ?".
                                "\n\n".
                                "*** PHP runtime information ***".
                                "\nPHP Version: ".phpversion().
                                "\nRunning on: ".php_uname().
                                "\nLoaded Extensions: ".print_array(get_loaded_extensions()).
                                "\nLDAP Module Version: ".get_module_setting('ldap','RCS Version').
                                "\n\n".
                                "*** PHP error information ***\n\n");
      }

    if (isset($_SESSION['js']) && $_SESSION['js']==FALSE){
      $error_collector= "<div>";
    } else {
      $error_collector= "
        <table summary=\"\" width=\"100%\" style='background-color:#E0E0E0;border-bottom:1px solid black;z-index:150;'>
          <tr>
            <td>";
      if (function_exists("get_template_path")){
              $error_collector.= "<img alt=\"\" align=\"middle\" src='".get_template_path('images/warning.png')."'>&nbsp;";
      }
      $error_collector.= "
              <font style='font-size:14px;font-weight:bold'>".
                _("Generating this page caused the PHP interpreter to raise some errors!")."
              </font>
            </td>
            <td align=right>
              <a href=\"mailto:gosa-bugs@oss.gonicus.de?subject=GOsa%20bugreport&amp;body=%BUGBODY%\">
                <img border='0' src='images/mailto.png' title='"._("Send bug report to the GOsa Team")."' class='center' alt=''>&nbsp;"._("Send bugreport")."
              </a>
            </td>
            <td align=right>
              <button onClick='toggle(\"errorbox\")'>".
                _("Toggle information")."
              </button>
            </td>
          </tr>
        </table>
        <div id='errorbox' style='position:absolute; z-index:150; visibility: hidden'>";
    }
  }
 
  /* Create error header */
  $error_collector.= "<table summary=\"\" width=\"100%\" cellspacing=0 style='background-color:#402005;color:white;border:2px solid red'><tr><td colspan=3><h1 style='color:white'>"._("PHP error")." \"$errstr\"</h1></td></tr>";
  
  $error_collector_mailto .= prepare4mailbody("=== Error === \n");
  $error_collector_mailto .= prepare4mailbody("PHP error: $errstr ($errfile, line $errline)\n");
  $error_collector_mailto .= prepare4mailbody("=== /Error === \n\n");

  /* Extract traceback data - if available */
  if (function_exists('debug_backtrace')){
    $trace= debug_backtrace();

    $error_collector_mailto .= prepare4mailbody("=== Trace ===");

    /* Generate trace history */
    for ($index= 0; $index<count($trace); $index++){


      $ct= $trace[$index];
      $loc= "";
      if (isset($ct['class'])){
        $loc.= _("class")." ".$ct['class'];
        if (isset($ct['function'])){
          $loc.= " / ";
        }
      }
      if (isset($ct['function'])){
        $loc.= _("function")." ".$ct['function'];
      }
      if (isset($ct['type'])){
        switch ($ct['type']){
          case "::":
            $type= _("static");
          break;

          case "->":
            $type= _("method");
          break;
        }
      } else {
        $type= "-";
      }
      $args= "";
      if (isset($ct['args'])){
        foreach ($ct['args'] as $arg){

          /* Avoid convertig object to string errors */ 
          if(is_object($arg)){
            $arg = "CLASS:&nbsp;".get_class($arg);
          }

          $args.= htmlentities("\"$arg\", ");
        }
      }
      $args= preg_replace("/, $/", "", $args);
      if ($args == ""){
        $args= "-";
      }
      if(isset($ct['file'])) {
        $file= $ct['file'];
      }else{
        $file="";
      }
      if(isset($ct['line'])) {
        $line= $ct['line'];
      }else{
        $line="";
      }
      $color= ($index&1)?'#404040':'606060';
      $error_collector.= "<tr style='background-color:$color'><td style='padding-left:20px' width=\"30%\">"._("Trace")."[$index]: $loc</td>";
      $error_collector.= "<td>"._("File").": $file ("._('Line')." $line)</td><td width=\"10%\">"._("Type").": $type</td></tr>";
      $error_collector.= "<tr style='background-color:$color'><td colspan=3 style='padding-left:20px;'>"._("Arguments").": $args</td></tr>";

      /* Add trace part to mailto body */
      $error_collector_mailto .= prepare4mailbody(
                                   "\nTrace[".$index."]:".$loc.
                                   "\nFile : ".$file.
                                   "\nLine : ".$line.
                                   "\nType : ".$type.
                                   "\n  ".$args.
                                   "\n");
      }
  }
  $error_collector_mailto .= prepare4mailbody("=== /Trace === \n");

  /* Close error table */
  $error_collector.= "</table>";

  /* Flush in case of fatal errors */
  if (preg_match('/^fatal/i', $errstr)){
    echo $error_collector."</div>";
    flush();
    exit;
  }
}


function prepare4mailbody($string)
{
  $string = html_entity_decode($string);

  $from = array(
                "/%/",
                "/ /",
                "/\n/",  
                "/\r/",
                "/!/",
                "/#/",
                "/\*/",
                "/\//",
                "/</",
                "/>/",
                "/\?/",
                "/\&/",
                "/\(/",
                "/\)/",
                "/\"/");
  
  $to = array(  
                "%25",
                "%20",
                "%0A",
                "%0D",
                "%21",
                "%23",
                "%2A",
                "%2F",
                "%3C",
                "%3E",
                "%3F",
                "%38",
                "%28",
                "%29",
                "%22");

  $string = preg_replace($from,$to,$string);

  return($string);
}


function dummy_error_handler()
{
}


/* Get base dir for reference */
$BASE_DIR= dirname(dirname(__FILE__));
$ROOT_DIR= $BASE_DIR."/html";
error_reporting (E_ALL);

/* Register error handler */
$error_collector= "";
$error_collector_mailto= "";
set_error_handler('gosaRaiseError');

$variables_order= "ES";
ini_set("register_globals",0);
ini_set("track_vars",1);
ini_set("display_errors",1);
ini_set("report_memleaks",1);
ini_set("include_path",".:$BASE_DIR/include");

/* Do smarty setup */
require("smarty/Smarty.class.php");
$smarty = new Smarty;
$smarty->template_dir = $BASE_DIR.'/ihtml/';
$smarty->caching= false;
$smarty->php_handling= SMARTY_PHP_REMOVE;

/* Set timezone */
if (function_exists("date_default_timezone_set")){
  date_default_timezone_set("GMT");
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>