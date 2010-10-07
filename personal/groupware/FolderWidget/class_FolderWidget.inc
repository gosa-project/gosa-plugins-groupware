<?php




class FolderWidget extends plugin
{
    public $config;
    private $FolderWidgetListing = NULL;
    private $folderList;

    private $baseEntryMap = array();

    function __construct(&$config)
    {
        $this->FolderWidgetListing = new FolderWidgetListing($this->config, get_userinfo());
    }


    /*! \brief  Tell the widget which folders should be displayed.
     */
    function setFolderList($arr)
    {
        $this->folderList = $this->buildRecursiveList($arr, '/');
        $this->FolderWidgetListing->setBase('cn=/');
        $this->FolderWidgetListing->setFolderRoot('cn=/');
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
        $data['cn=/'] = array(); 
        $data['cn=/']['name'] = '/';
        $data['cn=/']['status'] = '';
        $data['cn=/']['path'] = '/';
        $data['cn=/']['type'] = 'manual';
        $data['cn=/']['base'] = 'cn=/';
        $data['cn=/']['children'] = array();
        $ret['linear']['cn=/'] = &$data['cn=/'];

        $base = &$data['cn=/']['children'];
        $last = NULL;
        foreach($arr as $path => $entry){
            $current = &$base;
            $pathSteps = preg_split("/".preg_quote($seperator,'/')."/", $path);
            $entryPath = "";
            $entryBase = "cn=/";
            foreach($pathSteps as $name){
                $entryPath = (empty($entryPath)) ? $name : "{$entryPath}{$seperator}{$name}";
                $entryBase =  trim("cn={$name},{$entryBase}", ' ,');
                if(!isset($current[$entryPath])){
                    $current[$entryPath] = array();
                    $current[$entryPath]['children'] = array();
                    $current[$entryPath]['name'] = $name;
                    $current[$entryPath]['path'] = $path;
                    $current[$entryPath]['status'] = '';
                    $current[$entryPath]['type'] = 'manual';
                    $current[$entryPath]['base'] = $entryBase;
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
