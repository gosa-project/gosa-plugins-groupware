<div class="groupware-wrapper">
    <div class="row">
        <div class="col s12 xl6">
            <h3>{t}Groupware{/t}</h3>

            {render acl=$primaryMailAddressACL}
            <div class="input-field">
                <input type='text' name="primaryMailAddress" id="primaryMailAddress" value="{$primaryMailAddress}">
                <label for="useMailSizeLimit">{t}Mail address{/t}</label>
            </div>
            {/render}
            
            {if $mailSizeLimit_isActive}
            <hr class="divider">
            {render acl=$mailSizeLimitACL}
            <label>
                <input type='checkbox' name="useMailSizeLimit"
                    onClick="changeState('mailSizeLimit')" 
                    {if $useMailSizeLimit} checked {/if} value="1">
                <span></span>
            </label>
                <label for="useMailSizeLimit"></label>
            {/render}
            {render acl=$mailSizeLimitACL}
            <div class="input-field">
                <input type='text' name='mailSizeLimit' id="mailSizeLimit" 
                    {if $useMailSizeLimit} value="{$mailSizeLimit}" {else} value="" disabled {/if}>
                <label for="mailSizeLimit">{t}Use incoming mail size limitation{/t}</label>
            </div>
            {/render}
            {/if}

            
        </div>

        {if $alternateAddresses_isActive}
        <div class="col s12 xl6">
            <h3>{t}Alternative addresses{/t}</h3>
            {render acl=$alternateAddressesACL}
            <div class="input-field">
                <select id="alternateAddressList" style="width:100%;height:100px;" name="alternateAddressList[]" size="15" multiple
                    title="{t}List of alternative mail addresses{/t}">
                    {html_options values=$alternateAddresses output=$alternateAddresses}
                </select>
            </div>
            {/render}

            {render acl=$alternateAddressesACL}
            <div class="input-field add">
                <input type='text' name="alternateAddressInput">
                <button class="btn-small" type='submit' name='addAlternateAddress'>{msgPool type=addButton}</button>
                <button class="btn-small" type='submit' name='deleteAlternateAddress'>{msgPool type=delButton}</button>
            </div>
            {/render}
        </div>
        {/if}
    </div>
</div>

<input type="hidden" name="DistributionList_posted" value="1">
