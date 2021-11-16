<div class="folderEditDialog-wrapper">
    <div class="row">
        {if $isNew}
        <h3>{t}Create folder{/t}</h3>
        <table>
            <thead>
                <tr>
                    <th>{t}Name{/t}</th>
                </tr>
            </thead>
            <tbody>
                <tr>
                    <th>
                        <input name='folderName' id="folderName" value="{$folderItem.name}" type="text">
                    </th>
                </tr>
            </tbody>
        </table>

        {else}

        <h3>{t}Edit folder{/t}</h3>
        <table>
            <thead>
                <tr>
                    <th>:&nbsp;</th>
                    <th>{t}Path{/t}:&nbsp;</th>
                </tr>
            </thead>
            <tr>
                <td>{$folderName}</td>
                <td>{$folderPath}</td>
            </tr>
        </table>

        {/if}

        <hr class="divider">

        <h3>{t}Permissions{/t}</h3>
        
        <table class="stripped">
            <thead>
                <tr>
                    <th>{t}Type{/t}</th>
                    <th>{t}Name{/t}</th>
                    <th>{t}Permission{/t}</th>
                    <th></th>
                </tr>
            </thead>
            <tbody>
                {foreach from=$folderItem.acls item=item key=key}
                <tr>
                    <td>{$item.type}</td>
                    <td>
                        <div class="input-field">
                            <input type='text' name="permission_{$key}_name" value="{$item.name}">
                        </div>
                    </td>
                    <td>
                        {if $permissionCnt == 0 || !isset($permissions[$item.acl])}
                        <div class="input-field">
                            <input type='text' name="permission_{$key}_acl" value="{$item.acl}">
                        </div>
                        {else}
                        <div class="input-field">
                            <select name="permission_{$key}_acl" size=1>
                                {html_options options=$permissions selected=$item.acl}
                            </select>
                        </div>
                        {/if}
                    </td>
                    <td><button class="btn-small" name="permission_{$key}_del">{msgPool type=delButton}</button></td>
                </tr>
                {/foreach}
                <tr>
                    <td></td>
                    <td></td>
                    <td></td>
                    <td><button class="btn-small" name="permission_add">{msgPool type=addButton}</button></td>
                </tr>
            </tbody>
        </table>
    </div>
</div>

<div class='card-action'>
    <button class="btn-small" name="FolderEditDialog_ok">{msgPool type='okButton'}</button>
    <button class="btn-small" name="FolderEditDialog_cancel">{msgPool type='cancelButton'}</button>
</div>