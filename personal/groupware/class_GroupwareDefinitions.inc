<?php


# Access constants
if(!defined("GROUPWARE_LOOKUP")){
    define ("GROUPWARE_LOOKUP", 1);
    define ("GROUPWARE_READ", 2);
    define ("GROUPWARE_STATUS", 4);
    define ("GROUPWARE_WRITE", 8);
    define ("GROUPWARE_INSERT", 16);
    define ("GROUPWARE_POST", 32);
    define ("GROUPWARE_CREATE", 64);
    define ("GROUPWARE_DELETE", 128);
    define ("GROUPWARE_ADMINISTRATE", 256);

    define ("GROUPWARE_RIGHTS_NONE", 0);
    define ("RIGHTS_GROUPWARE_READ",  GROUPWARE_LOOKUP | GROUPWARE_READ | GROUPWARE_STATUS);
    define ("RIGHTS_GROUPWARE_POST", RIGHTS_GROUPWARE_READ | GROUPWARE_POST);
    define ("GROUPWARE_RIGHTS_APPEND", RIGHTS_GROUPWARE_POST | GROUPWARE_INSERT);
    define ("RIGHTS_GROUPWARE_WRITE", GROUPWARE_RIGHTS_APPEND | GROUPWARE_WRITE | GROUPWARE_CREATE | GROUPWARE_DELETE);
    define ("GROUPWARE_RIGHTS_ALL", RIGHTS_GROUPWARE_WRITE | GROUPWARE_ADMINISTRATE);
}

class GroupwareDefinitions
{
    public static function getPermissions()
    {
        $ret[GROUPWARE_RIGHTS_NONE] = _("None");
        $ret[RIGHTS_GROUPWARE_READ] = _("Read");
        $ret[RIGHTS_GROUPWARE_POST] = _("Post");
        $ret[GROUPWARE_RIGHTS_APPEND] = _("Append");
        $ret[RIGHTS_GROUPWARE_WRITE] = _("Write");
        $ret[GROUPWARE_RIGHTS_ALL] = _("All");
        return($ret);
    }
} 


?>
