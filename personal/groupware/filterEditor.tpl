<h3>{t}Available filter rules {/t}</h3>

{$list}
<select name='filterTemplate' size=1>
    <option>Mails von bestimmten Sendern in einen Ordner verschieben</option>
    <option>Mails von bekannten Sender (Adressbuch) in Ordner verschieben</option>
    <option>Mails löschen (Alle und immer!)</option>
    <option>Bla</option>
    <option>Manuelle Regel</option>
</select>
<button name='addFilter'>{msgPool type='addButton'}</button>


<hr>
<div class="plugin-actions">
    <button name='filterEditor_ok'>{msgPool type='okButton'}</button>
    <button name='filterEditor_cancel'>{msgPool type='cancelButton'}</button>
</div>
