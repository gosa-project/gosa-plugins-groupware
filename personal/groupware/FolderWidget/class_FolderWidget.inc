<?php

// Not yet finished need to be reviewed once the folder handling is working.


/*! \brief  A groupware folder widget which allow to navigate through a 
 *           given list of user or shared folders.
 *          Folders can be added or removed and ACLs assigned and updated.
 *
 *
 *  @param  Config  The GOsa configuration object
 *  @param  Array   The folder list to handle, e.g.
 *                  
 *                  $folders['shared/support']['status'] = ''; 
 *                  $folders['shared/support']['name'] = 'support'; 
 *                  $folders['shared/support']['acls'] = array();
 *
 *                  $folders['shared/support/2010']['status'] = 'added'; 
 *                  $folders['shared/support/2010']['name'] = '2010'; 
 *                  $folders['shared/support/2010']['acls'] = array('anne' => 1024, 'klaus' => '210');
 *                 
 *                  // 'status' Represents the folder status, maybe it wasn't touched then status is '';
 *                  //          If the folder was added, then the status is 'added'. Status 'removed' 
 *                  //          handles the folder as removed.
 *
 *                  // 'name'   Represents the entry name, this should match the name in the path.
 *
 *                  // 'acls'   Represents ACLs set for the folder.
 *                              Syntax  ['userNmae' => ACL] // See $this->setPermissions();
 *                              
 *
 */
class FolderWidget extends plugin
{
    public $pathTitle = "Folder";

    private $pathSeparator = "/";
    private $rootName = "/";
    private $fakeCnAttr = "cn=";
    private $rootBase = "";

    public $config;
    private $FolderWidgetListing = NULL;
    private $folderList;

    private $baseEntryMap = array();
    private $permissions = array();

    function __construct(&$config, $folderList, $accountPrefix)
    {
        $this->config = $config;

        // The rootBase is a dn-like representation of the root path
        // e.g.  rootPath is '/'  then is the rootBase 'cn=/'
        // These are just internally used variables and should not have an 
        //  effect on the results.
        $this->rootBase = $this->fakeCnAttr.$this->rootName;

        $this->accountPrefix = (empty($accountPrefix)) ? "" : rtrim($accountPrefix,$this->pathSeparator).$this->pathSeparator;
        $this->rootBase = $this->pathToDn($accountPrefix);

        // Update the folder list, initially.
        $this->setFolderList($folderList);
        $this->FolderWidgetListing = new FolderWidgetListing($this->config, get_userinfo(), $this->rootBase, $this);
        $this->FolderWidgetListing->setBase($this->rootBase);
        $this->FolderWidgetListing->setFolderRoot($this->rootBase);
    }

    function setPermissions($perms)
    {
        $this->permissions = $perms;
    }

    function pathToDn($path)
    {
        $dn ="cn=/";
        $list = preg_split("/".preg_quote($this->pathSeparator,'/')."/", $path,0,PREG_SPLIT_NO_EMPTY);
        foreach($list as $entry){
            $dn ="cn={$entry},{$dn}"; 
        }
        return($dn);
    }

    function getFolderType($item)
    {
        if(preg_match("/^user\//", $item['path'])) return('userFolder');
        if(preg_match("/^shared\//", $item['path'])) return('sharedFolder');
    }


    /*! \brief  Tell the widget which folders should be displayed.
     */
    function setFolderList($arr)
    {
        $this->folderList = $this->buildRecursiveList($arr, $this->pathSeparator);
    }


    /* \brief   Builds a hierarchical structure which can then be 
     *           used by the FolderWidgetListing.
     *
     * @param   Array   The folder list 
     * @return  Array   A hierarchical representation of the input array.
     */
    function buildRecursiveList($arr, $separator)
    {
        $data = array();
        $ret = array();

        // Add root Entry
        $data[$this->accountPrefix] = array(); 
        $data[$this->accountPrefix]['name'] = $this->rootName;
        $data[$this->accountPrefix]['status'] = '';
        $data[$this->accountPrefix]['acls'] = array();
        $data[$this->accountPrefix]['path'] = $this->accountPrefix;
        $data[$this->accountPrefix]['type'] = 'manual';
        $data[$this->accountPrefix]['base'] = $this->rootBase;
        $data[$this->accountPrefix]['parentPath'] = "";
        $data[$this->accountPrefix]['children'] = array();
        $this->baseEntryMap[$this->rootBase] = $this->accountPrefix;
        $ret['linear'][$this->accountPrefix] = &$data[$this->accountPrefix];

        $base = &$data[$this->accountPrefix]['children'];
        $last = NULL;
        foreach($arr as $path => $entry){

            // Only add entries that match the accountPrefix 
            //   e.g.  "/user/admin/"
            if(!preg_match("/^".preg_quote($this->accountPrefix,'/')."/", $path)) continue;

            $current = &$base;
            $pathSteps = preg_split("/".preg_quote($separator,'/')."/", $path);
            $entryPath = "";
            $parentPath = "";
            $entryBase = "";
            foreach($pathSteps as $name){
                $parentPath = $entryPath;
                $entryPath = (empty($entryPath)) ? $name : "{$entryPath}{$separator}{$name}";
                $entryBase =  $this->pathToDn($entryPath);

                echo $entryPath."<br>";

                // Add eventually missing subFolders, to enable the navigation 
                //  into these folders via the management lists.
                if(preg_match("/^".preg_quote($this->accountPrefix,'/').$this->accountPrefix, $entryPath)){
                    if(!isset($current[$entryPath])){
                        $current[$entryPath] = array();
                        $current[$entryPath]['children'] = array();
                        $current[$entryPath]['name'] = $name;
                        $current[$entryPath]['path'] = $path;
                        $current[$entryPath]['status'] = '';
                        $current[$entryPath]['acls'] = array();
                        $current[$entryPath]['type'] = 'manual';
                        $current[$entryPath]['base'] = $entryBase;
                        $current[$entryPath]['parentPath'] = $entryPath;
                        $current[$entryPath]['folderType'] = $this->getFolderType($current[$entryPath]);

                        $ret['linear'][$entryPath] = &$current[$entryPath];
                        $this->baseEntryMap[$entryBase] = $entryPath;
                    }
                    $last = &$current[$entryPath];
                    $current = &$current[$entryPath]['children'];
                }
            }
            $last['acls'] = $entry['acls'];
            $last['name'] = $entry['name'];
            $last['type'] = 'real';
            $last['status'] = $entry['status'];
            $ret['linear'][$path] = &$last;
        }
        $ret['recursive'] = $data;
        return($ret);
    }

    
    function editByBase($base)
    {
        $item = NULL;
        foreach($this->folderList['linear'] as $item){
            if($item['base'] == $base){
                break;
            }
        }
        if($item){
            $this->currentlyEditedItem = $item;
            $this->dialog = new FolderEditDialog($this->config, $item);
            $this->dialog->setPermissions($this->permissions);
        }
    }


    function removeByBase($base)
    {
        $this->_recursiveRemoveByBase($this->folderList['recursive'], $base);
        $this->_recursiveRemoveByBase($this->folderList['linear'], $base);
        $this->FolderWidgetListing->setFolderList($this->folderList);
    }


    function addFolder()
    {
            $this->currentlyEditedItem = NULL;
            $this->dialog = new FolderEditDialog($this->config, array(), TRUE);
            $this->dialog->setPermissions($this->permissions);
    }


    function _recursiveRemoveByBase(&$arr, $base)
    {
        $ret = array();
        foreach($arr as $path => $item){
            if($item['base'] == $base){

                // remove all children too
                if(isset($item['children']) && count($item['children'])){
                    foreach($item['children'] as $child){
                        $this->_recursiveRemoveByBase($this->folderList['recursive'], $child['base']);
                    }
                }

                $arr[$path]['status'] = 'removed';
            }elseif(isset($item['children']) && count($item['children'])){
                $ret = array_merge($ret, $this->_recursiveRemoveByBase($arr[$path]['children'], $base));
            }
        }
        return($ret);
        
    }

    function _saveRecursive($arr)
    {
        $ret = array();
        foreach($arr as $path => $item){
            if($item['type'] == 'real'){ 
                $ret[$path] = array(
                        'name' => $item['name'],
                        'status' => $item['status'],
                        'acls' => $item['acls'],
                        );
            }
            if(isset($item['children']) && count($item['children'])){
                $ret = array_merge($ret, $this->_saveRecursive($item['children']));
            }
        }
        return($ret);
    }

    function save()
    {
        return($this->_saveRecursive($this->folderList['recursive']));
    }

    function execute()
    {
        // Generate the listing output 
        session::set('FolderWidgetListing::folderList', $this->folderList);
        $this->FolderWidgetListing->setFolderList($this->folderList);
        $str = $this->FolderWidgetListing->execute();

        if($this->dialog){
            $this->dialog->save_object();
            return($this->dialog->execute());
        }

        // Return listing HTML
        return($str);
    }

    function save_object()
    {
        $this->FolderWidgetListing->save_object();

        // Handle dialogs 
        if(isset($_POST['FolderEditDialog_ok']) && is_object($this->dialog)){
    
            $this->dialog->save_object();
            $msgs = $this->dialog->check();
            if(count($msgs)){
                msg_dialog::displayChecks($msgs);
            }else{
                if($this->dialog->isNew()){
                    // Detect the current entry
                    $base = $this->FolderWidgetListing->getBase();
                    if(isset($this->baseEntryMap[$base])){
                        $entry = $this->folderList['linear'][$this->baseEntryMap[$base]];
                        $path = $entry['path'];
                        $newItem = $this->dialog->save();
                        $name = $newItem['name'];
                        $newPath = rtrim($path,$this->pathSeparator).$this->pathSeparator.$name;

                        $newItem['status'] = 'added';
                        $newItem['type'] = 'real';
                        $newItem['children'] = array();
                        $newItem['path'] = $newPath;
                        $newItem['parentPath'] = $entry['path'];
                        $newItem['folderType'] = $this->getFolderType($newItem);
                        $newItem['base'] = $this->pathToDn($newPath);
    
                        $this->folderList['linear'][$path]['children'][$newPath] = $newItem;
                        $this->folderList['linear'][$newPath] = &$this->folderList['linear'][$path]['children'][$newPath];

                        $this->baseEntryMap[$newItem['base']] = $newPath;
                    }
                }else{
                    $newItem = $this->dialog->save();
                    $path = $this->currentlyEditedItem['path'];
                    $this->folderList['linear'][$path]['acls'] = $newItem['acls']; 
                }
                $this->dialog = NULL;
            }
        }
        if(isset($_POST['FolderEditDialog_cancel'])){
            $this->dialog = NULL;
        }
    }
}

?>
