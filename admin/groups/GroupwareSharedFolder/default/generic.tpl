<div class="groupware-wrapper">
    <div class="row">
        <div class="col s12 xl6">
            <h3>{t}Groupware shared folder{/t}</h3>

            {render acl=$folderListACL}
                {t}Edit folder list{/t}&nbsp;<button class="btn-small" name='configureFolder'>{msgPool type=editButton}</button>
            {/render}
        </div>
    </div>
</div>

<input type="hidden" name="GroupwareSharedFolder_posted" value="1">
