<h3>{t}Filter editor{/t}</h3>

<table summary="{t}Generic settings{/t}">
    <tr>
        <td><LABEL for='name'>{t}Name{/t}</LABEL>:</td>
        <td><input type='text' id='name' name="DESC" value="{$NAME}"></td>
    </tr>
    <tr>
        <td><LABEL for='desc'>{t}Description{/t}:</LABEL></td>
        <td><input type='text' id='desc' name="DESC" value="{$DESC}"></td>
    </tr>
</table>

<hr>
{$filterStr}
<hr>
<div class="plugin-actions">
    <button name='filterEditor_ok'>{msgPool type='okButton'}</button>
    <button name='filterEditor_cancel'>{msgPool type='cancelButton'}</button>
</div>
