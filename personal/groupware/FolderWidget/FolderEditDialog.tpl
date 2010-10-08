<h3>{t}Folder{/t}</h3>

<table>
    <tr>
        <td>{t}Name{/t}:&nbsp;</td>
        <td>{$folderItem.name}</td>
    </tr>
    <tr>
        <td>{t}Path{/t}:&nbsp;</td>
        <td>{$folderItem.path}</td>
    </tr>
</table>

<hr>

<h3>{t}Permissions{/t}</h3>

<table>
    <tr>
        <td style='width:100px;'>{t}Type{/t}</td>
        <td style='width:180px;'>{t}Name{/t}</td>
        <td style='width:180px;'>{t}Permission{/t}</td>
    </tr>
    {foreach from=$folderItem.acls item=item key=key}
        <tr>
            <td>{$item.type}</td>
            <td><input type='text' name="permission_{$key}_name" value="{$item.name}"></td>
            <td><input type='text' name="permission_{$key}_acl" value="{$item.acl}"></td>
            <td><button name="permission_{$key}_del">{msgPool type=delButton}</button></td>
        </tr>
    {/foreach}
    <tr>
        <td></td>
        <td></td>
        <td></td>
        <td><button name="permission_add">{msgPool type=addButton}</button></td>
    </tr>
</table>

<hr>
<div class='plugin-actions'>
    <button name="FolderEditDialog_ok">{msgPool type='okButton'}</button>
    <button name="FolderEditDialog_cancel">{msgPool type='cancelButton'}</button>
</div>
