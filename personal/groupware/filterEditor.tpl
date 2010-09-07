<h3>{t}Filter editor{/t}</h3>

<table summary="{t}Generic settings{/t}">
    <tr>
        <td><LABEL for='NAME'>{t}Name{/t}</LABEL>:</td>
        <td><input style='width:300px;' type='text' id='NAME' name="NAME" value="{$NAME}"></td>
    </tr>
    <tr>
        <td><LABEL for='DESC'>{t}Description{/t}:</LABEL></td>
        <td><input style='width:300px;' type='text' id='DESC' name="DESC" value="{$DESC}"></td>
    </tr>
</table>

<hr>
{$filterStr}
<hr>
<div class="plugin-actions">
    <button name='filterEditor_ok'>{msgPool type='okButton'}</button>
    <button name='filterEditor_cancel'>{msgPool type='cancelButton'}</button>
</div>
<input type='hidden' value='1' name='filterEditorPosted'>
