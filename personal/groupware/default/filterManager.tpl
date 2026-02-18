<div class="filterManager-wrapper">
    <div class="row">
        <div class="col s12 xl6">
            <h3>{t}Available filter rules {/t}</h3>

            {$list}

            {render acl=$acl}
                <button class="btn-small" name='addFilter'>{msgPool type='addButton'}</button>
            {/render}
        </div>
    </div>

    <div class="card-action">
        {render acl=$acl}
            <button class="btn-small primary" name='filterManager_ok'>{msgPool type='okButton'}</button>
        {/render}
        <button class="btn-small primary" name='filterManager_cancel'>{msgPool type='cancelButton'}</button>
    </div>
</div>
