<div class="groupware-wrapper">
    <div class="row">
        {if $initFailed}
        <h3>{t}Communication with backend failed, please check the rpc connection and try again!{/t}</h3>
        <button class="btn-small" name="retry">{t}Retry{/t}</button>
        {elseif $rpcError}
        <h3>{t}Communication with backend failed, please check the rpc connection and try again!{/t}</h3>
        <button class="btn-small" name="retry">{t}Retry{/t}</button>
        {else}
        <div class="col s12 xl6">
            <h3>{t}Generic{/t}</h3>

            {render acl=$mailAddressACL}
            <div class="input-field">
                <input type='text' id="mailAddress" name="mailAddress" value="{$mailAddress}">
                <label for="mailAddress">{t}Primary address{/t}{$must}</label>
            </div>
            {/render}

            {if $mailLocations_isActive}
            {render acl=$mailLocationACL}
            <div class="input-field">
                <select size="1" id="mailLocation" name="mailLocation"
                    title="{t}Specify the location for the mail account{/t}">
                    {html_options values=$mailLocations output=$mailLocations selected=$mailLocation}
                </select>
                <label>{t}Account location{/t}</label>
            </div>
            {/render}
            {/if}

            {if $mailFolder_isActive}
            <h4>{t}Mail folder{/t}</h4>
            {if $uid == ""}
            <p>{t}Can only be set for existing accounts!{/t}</p>
            {else}
            {render acl=$mailFolderACL}
            <button class="btn-small" name='configureFolder'>{msgPool type=editButton}</button>
            {/render}
            {/if}
            {/if}

            {if $quotaUsage_isActive}
            {render acl=$quotaUsageACL}
            <div class="input-field">
                <input type='text' id='quotaUsage_dummy' name='quotaUsage_dummy' disabled value="{$quotaUsage}">
                <label for='quotaUsage_dummy'>{t}Quota usage{/t}</label></td>
            </div>
            {/render}
            {/if}

            {if $quotaSize_isActive}
            {render acl=$quotaSizeACL}
            <div class="input-field">
                <input type='text' id="quotaSize" name="quotaSize" value="{$quotaSize}"> MB
                <label for="quotaSize">{t}Quota size{/t}</label>
            </div>
            {/render}
            {/if}

            {if $mailFilter_isActive}
            {render acl=$mailFilterACL mode=read_active}
            <h4>{t}Mail filter{/t}</h4>
            <button class="btn-small" name='configureFilter'>{t}Configure filter{/t}</button>
            {/render}
            {/if}
        </div>

        <div class="col s12 xl6">
            {if !$alternateAddresses_isActive}
            <p></p>
            {else}
            <h3>>{t}Alternative addresses{/t}</h3>
            {render acl=$alternateAddressesACL}
            <div class="input-field">
                <select id="alternateAddressList" style="width:100%;height:100px;" name="alternateAddressList[]"
                    size="15" multiple title="{t}List of alternative mail addresses{/t}">
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
            {/if}
        </div>
    </div>

    <div class="row">
        <h3>{t}Spam filter configuration{/t}</h3>

        {if $vacationMessage_isActive || $forwardingAddresses_isActive}
        <hr class="divider">

        <div class="col s12 xl6">
            {if $vacationMessage_isActive}
            {render acl=$vacationEnabledACL}
            <label>
                <input type=checkbox name="vacationEnabled" value="1" {if $vacationEnabled} checked {/if}
                    id="vacationEnabled"
                    title="{t}Select to automatically response with the vacation message defined below{/t}"
                    class="center" onclick="changeState('vacationMessage');">
                <span>
                    <h3>{t}Vacation message{/t}</h3>
                </span>
            </label>
            {/render}

            <span>{t}Activate vacation message{/t}</span>

            {render acl=$vacationMessageACL}
            <div class="input-field">
                <textarea id="vacationMessage" style="width:99%; height:100px;" {if !$vacationEnabled} disabled {/if}
                    name="vacationMessage" rows="4" cols="512">{$vacationMessage}</textarea>
            </div>
            {/render}

            {if $displayTemplateSelector eq "true"}
            {render acl=$vacationMessageACL}
            <div class="input-field add">
                <select id='vacation_template' name="vacation_template" size=1>
                    {html_options options=$vacationTemplates selected=$vacationTemplate}
                </select>
                <button class="btn-small" type='submit' name='import_vacation'
                    id="import_vacation">{t}Import{/t}</button>
            </div>
            {/render}
            {/if}
        </div>
        {/if}

        {if $forwardingAddresses_isActive}
        <div class="col s12 xl6">
            {render acl=$forwardingAddressesACL}
            <div class="input-field">
                <select id="forwardingAddressList" style="width:100%; height:100px;" name="forwardingAddressList[]"
                    size=15 multiple>
                    {html_options values=$forwardingAddresses output=$forwardingAddresses}
                </select>
                <label>{t}Forward messages to{/t}</label>
            </div>
            {/render}

            {render acl=$forwardingAddressesACL}
            <div class="input-field add">
                <input type='text' id='forwardingAddressInput' name="forwardingAddressInput">
                <button class="btn-small" type='submit' name='addForwardingAddress' id="addForwardingAddress">{msgPool
                    type=addButton}</button>&nbsp;
                <button class="btn-small" type='submit' name='addLocalForwardingAddress'
                    id="addLocalForwardingAddress">{t}Add local{/t}</button>&nbsp;
                <button class="btn-small" type='submit' name='deleteForwardingAddress'
                    id="deleteForwardingAddress">{msgPool
                    type=delButton}</button>
            </div>
            {/render}
        </div>
        {/if}

        {/if}
    </div>

    {* Do not render the Flag list while there are none! *}
    {if $mailBoxWarnLimit_isActive || $mailBoxSendSizelimit_isActive ||
    $mailBoxHardSizelimit_isActive || $mailBoxAutomaticRemoval_isActive ||
    $localDeliveryOnly_isActive || $dropOwnMails_isActive}
    <div class="row">
        <h3>{t}Mailbox options{/t}</h3>
        {if $mailBoxWarnLimit_isActive}
        <div class="col s12 xl6">
            {render acl=$mailBoxWarnLimitACL}
            <label>
                <input id='mailBoxWarnLimitEnabled' value='1' name="mailBoxWarnLimitEnabled"
                    onclick="changeState('mailBoxWarnLimitValue');" value="1" {if $mailBoxWarnLimitEnabled} checked
                    {/if} class="center" type='checkbox'>
                <span></span>
            </label>
            {/render}

            {render acl=$mailBoxWarnLimitACL}
            <div class="input-value">
                <input id="mailBoxWarnLimitValue" name="mailBoxWarnLimitValue" size="6" align="middle" type='text'
                    value="{$mailBoxWarnLimitValue}" {if !$mailBoxWarnLimitEnabled} disabled {/if} class="center">
                <label for="mailBoxWarnLimitValue">{t}Warn user about a full mailbox when it reaches{/t}
                    ({t}MB{/t})</label>
            </div>
            {/render}
        </div>
        {/if}

        {if $mailBoxSendSizelimit_isActive}
        {render acl=$mailBoxSendSizelimitACL}
        <label>
            <input id='mailBoxSendSizelimitEnabled' value='1' name="mailBoxSendSizelimitEnabled"
                onclick="changeState('mailBoxSendSizelimitValue');" value="1" {if $mailBoxSendSizelimitEnabled} checked
                {/if} class="center" type='checkbox'>
            <span></span>
        </label>
        {/render}

        {render acl=$mailBoxSendSizelimitACL}
        <div class="input-field">
            <input id="mailBoxSendSizelimitValue" name="mailBoxSendSizelimitValue" size="6" align="middle" type='text'
                value="{$mailBoxSendSizelimitValue}" {if !$mailBoxSendSizelimitEnabled} disabled {/if} class="center">
            <label for="mailBoxSendSizelimitValue">{t}Refuse incoming mails when mailbox size reaches{/t}
                ({t}MB{/t})</label>
        </div>
        {/render}
        {/if}

        {if $mailBoxHardSizelimit_isActive}
        {render acl=$mailBoxHardSizelimitACL}
        <label>
            <input id='mailBoxHardSizelimitEnabled' value='1' name="mailBoxHardSizelimitEnabled"
                onclick="changeState('mailBoxHardSizelimitValue');" value="1" {if $mailBoxHardSizelimitEnabled} checked
                {/if} class="center" type='checkbox'>
            <span></span>
        </label>
        {/render}

        {render acl=$mailBoxHardSizelimitACL}
        <div class="input-field">
            <input id="mailBoxHardSizelimitValue" name="mailBoxHardSizelimitValue" size="6" align="middle" type='text'
                value="{$mailBoxHardSizelimitValue}" {if !$mailBoxHardSizelimitEnabled} disabled {/if} class="center">

            <label for="mailBoxHardSizelimitValue">{t}Refuse to send and receive mails when mailbox size
                reaches{/t} ({t}MB{/t})</label>
        </div>
        {/render}
        {/if}

        {if $mailBoxAutomaticRemoval_isActive}
        {render acl=$mailBoxAutomaticRemovalACL}
        <label>
            <input id='mailBoxAutomaticRemovalEnabled' value='1' name="mailBoxAutomaticRemovalEnabled"
                onclick="changeState('mailBoxAutomaticRemovalValue');" value="1" {if $mailBoxAutomaticRemovalEnabled}
                checked {/if} class="center" type='checkbox'>
            <span></span>
        </label>
        {/render}

        {render acl=$mailBoxAutomaticRemovalACL}
        <div class="input-field">
            <input id="mailBoxAutomaticRemovalValue" name="mailBoxAutomaticRemovalValue" size="6" align="middle"
                type='text' value="{$mailBoxAutomaticRemovalValue}" {if !$mailBoxAutomaticRemovalEnabled} disabled {/if}
                class="center">
            <label for="mailBoxAutomaticRemovalValue">{t}Remove mails older than {/t} ({t}days{/t})</label>
        </div>
        {/render}
        {/if}

        {if $mailLimit_isActive}
        <label>
            <input id='mailLimitReceiveEnabled' value='1' name="mailLimitReceiveEnabled" value="1"
                onclick="changeState('mailLimitReceiveValue');" {if $mailLimitReceiveEnabled} checked {/if}
                class="center" type='checkbox'>
            <span></span>
        </label>

        <div class="input-field">
            <input id="mailLimitReceiveValue" name="mailLimitReceiveValue" size="6" align="middle" type='text'
                value="{$mailLimitReceiveValue}" {if !$mailLimitReceiveEnabled} disabled {/if} class="center">
            <label for="mailLimit">{t}Mailbox size limits receiving mails{/t} ({t}kbyte{/t})</label>
        </div>

        <label>
            <input id='mailLimitSendEnabled' value='1' name="mailLimitSendEnabled" value="1"
                onclick="changeState('mailLimitSendValue');" {if $mailLimitSendEnabled} checked {/if} class="center"
                type='checkbox'>
            <span></span>
        </label>

        <div class="input-field">
            <input id="mailLimitSendValue" name="mailLimitSendValue" size="6" align="middle" type='text'
                value="{$mailLimitSendValue}" {if !$mailLimitSendEnabled} disabled {/if} class="center">
            <label for="mailLimit">{t}Mailbox size limits sending mails{/t} ({t}kbyte{/t})</label>
        </div>
        {/if}

        {if $localDeliveryOnly_isActive}
        {render acl=$localDeliveryOnlyACL}
        <label>
            <input id='localDeliveryOnly' type=checkbox name="localDeliveryOnly" value="1" {if $localDeliveryOnly}
                checked {/if} title="{t}Select if user can only send and receive inside his own domain{/t}"
                class="center">
            <span>{t}User is only allowed to send and receive local mails{/t}</span>
        </label>
        {/render}

        {/if}

        {if $dropOwnMails_isActive}
        {render acl=$dropOwnMailsACL}
        <label>
            <input id='dropOwnMails' type=checkbox name="dropOwnMails" value="1" {if $dropOwnMails} checked {/if}
                title="{t}Select if you want to forward mails without getting own copies of them{/t}">
            <span>{t}No delivery to own mailbox{/t}</span>
        </label>
        {/render}
        {/if}
        {/if}
    </div>
    {/if}
</div>

<input type='hidden' name='groupwarePluginPosted' value='1'>