<?php
/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2003-2010 GONICUS GmbH
 *
 * ID: $$Id: class_listing.inc 15296 2010-01-26 08:27:39Z cajus $$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

class baseSelector {

  private $base;
  private $pid;
  private $action;
  private $submitButton= true;
  protected $tree;
  protected $pathMapping;

  // It would be better to get a dn -> [name, description, type] array
  // to avoid these tasks be done several times. Skipping for the moment.
  #'dc' => 'plugins/departments/images/domain.png',
  #'dc' => 'plugins/departments/images/dc.png',
  #'l' => 'plugins/departments/images/locality.png',
  #'c' => 'plugins/departments/images/country.png',
  #'o' => 'plugins/departments/images/organization.png',
  #'ou' => 'plugins/departments/images/folder.png',

  function __construct($bases, $base= "")
  {
    // Initialize pid
    $this->pid= preg_replace("/[^0-9]/", "", microtime(TRUE));

    // Transfer data
    $this->setBases($bases);
    $this->setBase($base);
  }


  function setSubmitButton($flag)
  {
    $this->submitButton= $flag;
  }


  function setBase($base)
  {
    if (isset($this->pathMapping[$base])) {
      $this->base= $base;
      $this->update(true);
    }
  }


  function setBases($bases)
  {
    global $config;

    $this->pathMapping= array();
    $selected= $this->base == $config->current['BASE']?"Selected":"";
    $first= true;
    $last_indent= 2;

    foreach ($bases as $base => $dummy) {

      // Build path style display
      $elements= explode(',', substr($base, 0, strlen($base) - strlen($config->current['BASE'])));
      $elements= array_reverse($elements, true);

      $this->pathMapping[$base]= $base == $config->current['BASE']? '/' : ldap::fix(preg_replace('/(^|,)[a-z0-9]+=/i', '/', implode(',', $elements)));
    }

    // Save bases to session for autocompletion
    session::global_set('pathMapping', $this->pathMapping);
  }


  function update($force= false)
  {
    global $config;

    // Analyze for base changes if needed
    $this->action= null;
    $last_base= $this->base;
    if(isset($_REQUEST['PID']) && $_REQUEST['PID'] == $this->pid) {
      if (isset($_REQUEST['REBASE'])) {
        $new_base= base64_decode($_REQUEST['REBASE']);
        if (isset($this->pathMapping[$new_base])) {
          $this->base= $new_base;
          $this->action= 'rebase';
        } else {
          die ("Base mismatch!");
        }
      }
    } elseif (isset($_POST['bs_input_'.$this->pid])) {

      // Check if base is available
      foreach ($this->pathMapping as $key => $path) {
        if (mb_strtolower($path) == mb_strtolower($_POST['bs_input_'.$this->pid])) {
          $this->base= $key;
          break;
        }
      }

    }

    /* Skip if there's no change */
    if (($this->tree && $this->base == $last_base) && !$force) {
      return;
    }

    $link= "href='?plug=".$_GET['plug']."&amp;PID=".$this->pid."&amp;REBASE=".base64_encode($config->current['BASE'])."'";
    $this->tree= "<input type='text' size='35' name='bs_input_".$this->pid."' id='bs_input_".$this->pid."' onkeydown=\"\$('bs_".$this->pid."').hide()\" onfocus=\"\$('bs_".$this->pid."').hide()\" onmouseover=\"Element.clonePosition(\$('bs_".$this->pid."'), 'bs_input_".$this->pid."', {setHeight: false, setWidth: false, offsetTop:(Element.getHeight('bs_input_".$this->pid."'))});\$('bs_".$this->pid."').show();\" onmouseout=\"rtimer= Element.hide.delay(0.25, 'bs_".$this->pid."')\" value='".$this->pathMapping[$this->base]."'>";


    // Autocompleter
    $this->tree.= "<div id='autocomplete_".$this->pid."' class='autocomplete'></div>".
                  "<script type='text/javascript'>".
                  "new Ajax.Autocompleter('bs_input_".$this->pid."', 'autocomplete_".$this->pid."', 'autocomplete.php?type=base', { minChars: 3, frequency: 0.5 });".
                  "$('bs_input_".$this->pid."').observe('keypress', function(event) { if(event.keyCode == Event.KEY_RETURN) { $('submit_base_".$this->pid."').click(); } });".
                  "</script>";

    $selected= $this->base == $config->current['BASE']?"Selected":"";
    $this->tree.= "<div class='treeList' style='display:none' id='bs_".$this->pid."' onmouseover=\"window.clearTimeout(rtimer);\" onmouseout=\"rtimer= Element.hide.delay(0.25, 'bs_".$this->pid."')\"><a class='treeList$selected' $link>/ ["._("Root")."]</a><ul class='treeList'>\n";
    $first= true;
    $last_indent= 2;

    foreach ($this->pathMapping as $base => $dummy) {

      // Skip root for tree
      if ($base == $config->current['BASE']) {
        continue;
      }

      // Build path style display
      $elements= explode(',', substr($base, 0, strlen($base) - strlen($config->current['BASE'])));

      $indent= count($elements);
      if (!$first && ($indent == $last_indent)) {
        $this->tree.= "</li>\n";
      }
      if ($indent > $last_indent) {
        $this->tree.= "<ul>\n";
      }
      if ($indent < $last_indent) {
        for ($i= 0; $i < ($last_indent-$indent); $i++) {
          $this->tree.= "</li></ul>\n";
        }
        $this->tree.= "</li>\n";
      }
      $selected= $this->base == $base?" class='treeListSelected'":"";
      $link= "href='?plug=".$_GET['plug']."&amp;PID=".$this->pid."&amp;REBASE=".base64_encode($base)."'";
      $this->tree.= "<li><a$selected $link>".str_replace(' ', '&nbsp;', ldap::fix(preg_replace('/^[a-z0-9]+=([^,]+),.*$/i', '$1', $base)))."</a>";

      $last_indent= $indent;
      $first= false;
    }

    // Close tree
    for ($i= 0; $i<$last_indent; $i++) {
      $this->tree.= "</li></ul>\n";
    }
    $this->tree.= "</div>\n";

    // Draw submitter if required
    if ($this->submitButton) {
      $this->tree.= "&nbsp;<input class='center' type='image' src='images/lists/submit.png' align='middle' title='"._("Submit")."' name='submit_base_".$this->pid."' id='submit_base_".$this->pid."' alt='"._("Submit")."'>";
    }

  }

  function render()
  {
    return $this->tree;
  }


  function getBase()
  {
    return $this->base;
  }


  function getAction()
  {
    // Do not do anything if this is not our PID, or there's even no PID available...
    if(!isset($_REQUEST['PID']) || $_REQUEST['PID'] != $this->pid) {
      return;
    }

    if ($this->action) {
      return array("targets" => array($this->base), "action" => $this->action);
    }

    return null;
  }

}

?>