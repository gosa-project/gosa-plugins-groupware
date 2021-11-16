<div class="filterEditor-wrapper">
    <div class="row">
        <div class="col s12 xl6">
            <h3>{t}Filter editor{/t}</h3>

            {render acl=$acl}
            <div class="input-field">
                <input type='text' id='NAME' name="NAME" value="{$NAME}">
                <label for='NAME'>{t}Name{/t}</label>
            </div>
            {/render}
            
            {render acl=$acl}
            <div class="input-field">
                <input type='text' id='DESC' name="DESC" value="{$DESC}">
                <label for='DESC'>{t}Description{/t}</label>
            </div>
            {/render}

            <hr class="divider">

            {render acl=$acl}
                {$filterStr}
            {/render}
                    
            <hr class="divider">
        </div>
    </div>

    <div class="card-action">
        {render acl=$acl}
            <button class="btn-small primary" name='filterEditor_ok'>{msgPool type='okButton'}</button>
        {/render}
        <button class="btn-small primary" name='filterEditor_cancel'>{msgPool type='cancelButton'}</button>
    </div>
    
    <input type='hidden' value='1' name='filterEditorPosted'>
</div>

