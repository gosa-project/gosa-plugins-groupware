<?php




class FolderWidget extends plugin
{

    private $pathSeperator = "/";
    private $rootName = "/";
    private $fakeCnAttr = "cn=";
    private $rootBase = "";

    public $config;
    private $FolderWidgetListing = NULL;
    private $folderList;

    private $baseEntryMap = array();

    function __construct(&$config)
    {
        $this->rootBase = $this->fakeCnAttr.$this->rootName;
        $this->FolderWidgetListing = new FolderWidgetListing($this->config, get_userinfo(), $this->rootBase);
        $this->FolderWidgetListing->setBase($this->rootBase);
        $this->FolderWidgetListing->setFolderRoot($this->rootBase);
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
        $this->folderList = $this->buildRecursiveList($arr, $this->pathSeperator);
    }


    /* \brief   Builds a hierarchical structure which can then be 
     *           used by the FolderWidgetListing.
     *
     * @param   Array   The folder list 
     * @return  Array   A hierarchical representation of the input array.
     */
    function buildRecursiveList($arr, $seperator)
    {
        $data = array();
        $ret = array();

        // Add root Entry
        $data[$this->rootBase] = array(); 
        $data[$this->rootBase]['name'] = $this->rootName;
        $data[$this->rootBase]['status'] = '';
        $data[$this->rootBase]['acls'] = array();
        $data[$this->rootBase]['path'] = $this->rootName;
        $data[$this->rootBase]['type'] = 'manual';
        $data[$this->rootBase]['base'] = $this->rootBase;
        $data[$this->rootBase]['parentPath'] = "";
        $data[$this->rootBase]['children'] = array();
        $ret['linear'][$this->rootBase] = &$data[$this->rootBase];

        $base = &$data[$this->rootBase]['children'];
        $last = NULL;
        foreach($arr as $path => $entry){
            $current = &$base;
            $pathSteps = preg_split("/".preg_quote($seperator,'/')."/", $path);
            $entryPath = "";
            $parentPath = "";
            $entryBase = $this->rootBase;
            foreach($pathSteps as $name){
                $parentPath = $entryPath;
                $entryPath = (empty($entryPath)) ? $name : "{$entryPath}{$seperator}{$name}";
                $entryBase =  trim("cn={$name},{$entryBase}", ' ,');
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
            $last['acls'] = $entry['acls'];
            $last['name'] = $entry['name'];
            $last['type'] = 'real';
            $last['status'] = $entry['status'];
            $ret['linear'][$path] = &$last;
        }
        $ret['recursive'] = $data;
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
        session::set('FolderWidgetListing::folderList', $this->folderList);
        $this->FolderWidgetListing->setFolderList($this->folderList);
        return($this->FolderWidgetListing->execute());
    }

    function save_object()
    {
        $this->FolderWidgetListing->save_object();
    }
}

?>
