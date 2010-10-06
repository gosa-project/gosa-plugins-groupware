{if $initFailed}
    <h3>{t}Communication with backend failed, please check the rpc connection and try again!{/t}</h3>
    <button name="retry">{t}Retry{/t}</button>
{elseif $rpcError}
    <h3>{t}Communication with backend failed, please check the rpc connection and try again!{/t}</h3>
    <button name="retry">{t}Retry{/t}</button>
{else}

{if $mailFolder_isActive}
    {$fileBrowser}

    {t}Add folder{/t}:&nbsp;
    <input name='folderName_Input' type='text'><button name="folderName_Add">{msgPool type=addButton}</button>

    {if $currentSelectedFolder != ""}
    <b>{t}Folder permission{/t}</b><br> 

    <p>
    {t}Current folder{/t}: <i>{$currentSelectedFolder}</i> <!--<input type='text' name="currentFolderName" value="{$folderEntries.name}">-->
    <b>{$folderEntries.name}</b>
    </p>

    <table>
        <tr>
            <td>{t}Name{/t}</td>
            <td>{t}Permission{/t}</td>
            <td>{t}Type{/t}</td>
        </tr>
        {foreach from=$folderEntries.acls item=item key=key}
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
    {/if}
{/if}
<table summary="{t}Mail settings{/t}" style='width:100%;'>
    <tr>
        <td style='width:50%; '>  
            <h3>{t}Generic{/t}</h3>
            <table summary="{t}Mail address configuration{/t}">
                <tr>
                    <td><label for="mailAddress">{t}Primary address{/t}</label>{$must}</td>
                    <td>
                        {render acl=$mailAddressACL}
                            <input type='text' id="mailAddress" name="mailAddress" value="{$mailAddress}">
                        {/render}
                    </td>
                </tr>
                <tr>
                    <td><label for="mailLocation">{t}Account location{/t}</label></td>
                    <td>
                        {render acl=$mailLocationACL}
                            <select size="1" id="mailLocation" name="mailLocation" 
                                title="{t}Specify the location for the mail account{/t}">
                                {html_options values=$mailLocations output=$mailLocations selected=$mailLocation}
                                <option disabled>&nbsp;</option>
                            </select>
                        {/render}
                    </td>
                </tr>
                {if $quotaUsage_isActive}
                <tr>
                    <td><label for='quotaUsage_dummy'>{t}Quota usage{/t}</label></td>
                    <td>
                        {render acl=$quotaUsageACL}
                            <input type='text' id='quotaUsage_dummy' name='quotaUsage_dummy' disabled value="{$quotaUsage}">
                        {/render}
                    </td>
                </tr>
                {/if}
                {if $quotaSize_isActive}
                <tr>
                    <td><label for="quotaSize">{t}Quota size{/t}</label></td>
                    <td>
                        {render acl=$quotaSizeACL}
                            <input type='text' id="quotaSize" name="quotaSize" value="{$quotaSize}"> MB
                        {/render}
                    </td>
                </tr>
                {/if}
                {if $mailFilter_isActive}
                <tr>
                    <td><label for="mailFilter">{t}Mail filter{/t}</label></td>
                    <td>
                        {render acl=$mailFilterACL mode=read_active}
                            <button name='configureFilter'>{t}Configure filter{/t}</button>
                        {/render}
                    </td>
                </tr>
                {/if}
            </table>

        </td>
        
        {if !$alternateAddresses_isActive}
            <td></td>
        {else}
            <td class='left-border'>&nbsp;</td>
            <td>
                <h3><label for="alternateAddressList">{t}Alternative addresses{/t}</label></h3>
                {render acl=$alternateAddressesACL}
                    <select id="alternateAddressList" style="width:100%;height:100px;" name="alternateAddressList[]" size="15" multiple
                        title="{t}List of alternative mail addresses{/t}">
                        {html_options values=$alternateAddresses output=$alternateAddresses}
                        <option disabled>&nbsp;</option>
                    </select>
                    <br>
                {/render}
                {render acl=$alternateAddressesACL}
                    <input type='text' name="alternateAddressInput">
                {/render}
                {render acl=$alternateAddressesACL}
                    <button type='submit' name='addAlternateAddress'>{msgPool type=addButton}</button>
                {/render}
                {render acl=$alternateAddressesACL}
                    <button type='submit' name='deleteAlternateAddress'>{msgPool type=delButton}</button>
                {/render}
            </td>
        {/if}
    </tr>
</table>

{if $vacationMessage_isActive || $forwardingAddresses_isActive}
<hr> 
    <table>
        <tr>
            <td style='width:50%'>

                {if $vacationMessage_isActive}
                <h3><label for="vacationMessage">{t}Vacation message{/t}</label></h3>

                <table summary="{t}Spam filter configuration{/t}">
                    <tr>
                        <td style='width:20px;'>
                            {render acl=$vacationEnabledACL}
                            <input type=checkbox name="vacationEnabled" value="1" 
                                {if $vacationEnabled} checked {/if}
                                id="vacationEnabled" 
                                title="{t}Select to automatically response with the vacation message defined below{/t}" 
                                class="center" 
                                onclick="changeState('vacationStart'); changeState('vacationStop'); changeState('vacationMessage');">
                            {/render}
                        </td>
                        <td colspan="4">
                            {t}Activate vacation message{/t}
                        </td>
                    </tr>
                    <tr>
                        <td>&nbsp;</td>
                        <td style='width:30px;'>{t}from{/t}</td>
                        <td>
                            {render acl=$vacationStartACL}
                                <input type="text" id="vacationStart" name="vacationStart" class="date" 
                                    style='width:100px' value="{$vacationStart}"
                                    {if !$vacationEnabled} disabled {/if}
                                    >
                            {/render}
                            {if $vacationStartACL|regex_replace:"/[cdmr]/":"" == "w"}
                                <script type="text/javascript">
                                    {literal}
                                        var datepicker  = new DatePicker({ 
                                            relative : 'vacationStart', 
                                            language : '{/literal}{$lang}{literal}', 
                                            keepFieldEmpty : true, 
                                            enableCloseEffect : false, 
                                            enableShowEffect : false });
                                    {/literal}
                                </script>
                            {/if}
                        </td>
                        <td style='width:30px;'>{t}till{/t}</td>
                        <td>
                            {render acl=$vacationStartACL}
                                <input type="text" id="vacationStop" name="vacationStop" class="date" 
                                    style='width:100px' value="{$vacationStop}"i
                                    {if !$vacationEnabled} disabled {/if}
                                    >
                            {/render}
                            {if $vacationStopACL|regex_replace:"/[cdmr]/":"" == "w"}
                                <script type="text/javascript">
                                    {literal}
                                        var datepicker2  = new DatePicker({ 
                                            relative : 'vacationStop', 
                                            language : '{/literal}{$lang}{literal}', 
                                            keepFieldEmpty : true, 
                                            enableCloseEffect : false,  
                                            enableShowEffect : false });
                                    {/literal}
                                </script>
                            {/if}
                        </td>
                    </tr>
                    <tr>
                        <td colspan=5>
                            {render acl=$vacationMessageACL}
                            <textarea id="vacationMessage" style="width:99%; height:100px;" 
                                {if !$vacationEnabled} disabled {/if}
                                name="vacationMessage" rows="4" cols="512">{$vacationMessage}</textarea>
                            {/render}
                            <br>
                            {if $displayTemplateSelector eq "true"}
                                {render acl=$vacationMessageACL}
                                    <select id='vacation_template' name="vacation_template" size=1>
                                        {html_options options=$vacationTemplates selected=$vacationTemplate}
                                        <option disabled>&nbsp;</option>
                                    </select>
                                {/render}
                                {render acl=$vacationMessageACL}
                                    <button type='submit' name='import_vacation' id="import_vacation">{t}Import{/t}</button>
                                {/render}
                            {/if}
                        </td>
                    </tr>
                </table>

                {/if}

            </td>
            {if !$forwardingAddresses_isActive}
                <td></td>
            {else}
                <td class='left-border'>&nbsp;</td>
                <td>
                    <h3><label for="forwardingAddressList">{t}Forward messages to{/t}</label></h3>
                    {render acl=$forwardingAddressesACL}
                        <select id="forwardingAddressList" style="width:100%; height:100px;" 
                            name="forwardingAddressList[]" size=15 multiple>
                            {html_options values=$forwardingAddresses output=$forwardingAddresses}
                            <option disabled>&nbsp;</option>
                        </select>
                    {/render}
                    <br>
                    {render acl=$forwardingAddressesACL}
                        <input type='text' id='forwardingAddressInput' name="forwardingAddressInput">
                    {/render}
                    {render acl=$forwardingAddressesACL}
                        <button type='submit' name='addForwardingAddress' 
                            id="addForwardingAddress">{msgPool type=addButton}</button>&nbsp;
                    {/render}
                    {render acl=$forwardingAddressesACL}
                        <button type='submit' name='addLocalForwardingAddress' 
                            id="addLocalForwardingAddress">{t}Add local{/t}</button>&nbsp;
                    {/render}
                    {render acl=$forwardingAddressesACL}
                        <button type='submit' name='deleteForwardingAddress' 
                            id="deleteForwardingAddress">{msgPool type=delButton}</button>
                    {/render}
                </td>
            {/if}
        </tr>
    </table>
{/if}
    
{* Do not render the Flag list while there are none! *}
{if $mailBoxWarnLimit_isActive || $mailBoxSendSizelimit_isActive ||
    $mailBoxHardSizelimit_isActive || $mailBoxAutomaticRemoval_isActive ||
    $localDeliveryOnly_isActive || $dropOwnMails_isActive}

<hr>
<h3>{t}Mailbox options{/t}</h3>
<table summary="{t}Flags{/t}">
    {if $mailBoxWarnLimit_isActive}
    <tr>
        <td>
            {render acl=$mailBoxWarnLimitACL}
                <input id='mailBoxWarnLimitEnabled' value='1' name="mailBoxWarnLimitEnabled" value="1" 
                    {if $mailBoxWarnLimitEnabled} checked {/if} class="center" type='checkbox'>
            {/render}
            <label for="mailBoxWarnLimitValue">{t}Warn user about a full mailbox when it reaches{/t}</label>
            {render acl=$mailBoxWarnLimitACL}
                <input id="mailBoxWarnLimitValue" name="mailBoxWarnLimitValue" 
                    size="6" align="middle" type='text' value="{$mailBoxWarnLimitValue}"  class="center"> {t}MB{/t}
            {/render}
        </td>
    </tr>
    {/if}
    {if $mailBoxSendSizelimit_isActive}
    <tr>
        <td>
            {render acl=$mailBoxSendSizelimitACL}
                <input id='mailBoxSendSizelimitEnabled' value='1' name="mailBoxSendSizelimitEnabled" value="1" 
                    {if $mailBoxSendSizelimitEnabled} checked {/if} class="center" type='checkbox'>
            {/render}
            <label for="mailBoxSendSizelimitValue">{t}Refuse incoming mails when mailbox size reaches{/t}</label>
            {render acl=$mailBoxSendSizelimitACL}
                <input id="mailBoxSendSizelimitValue" name="mailBoxSendSizelimitValue" 
                    size="6" align="middle" type='text' value="{$mailBoxSendSizelimitValue}"  class="center"> {t}MB{/t}
            {/render}
        </td>
    </tr>
    {/if}
    {if $mailBoxHardSizelimit_isActive}
    <tr>
        <td>
            {render acl=$mailBoxHardSizelimitACL}
                <input id='mailBoxHardSizelimitEnabled' value='1' name="mailBoxHardSizelimitEnabled" value="1" 
                    {if $mailBoxHardSizelimitEnabled} checked {/if} class="center" type='checkbox'>
            {/render}
            <label for="mailBoxHardSizelimitValue">{t}Refuse to send and receive mails when mailbox size reaches{/t}</label>
            {render acl=$mailBoxHardSizelimitACL}
                <input id="mailBoxHardSizelimitValue" name="mailBoxHardSizelimitValue" 
                    size="6" align="middle" type='text' value="{$mailBoxHardSizelimitValue}"  class="center"> {t}MB{/t}
            {/render}
        </td>
    </tr>
    {/if}
    {if $mailBoxAutomaticRemoval_isActive}
    <tr>
        <td>
            {render acl=$mailBoxAutomaticRemovalACL}
                <input id='mailBoxAutomaticRemovalEnabled' value='1' name="mailBoxAutomaticRemovalEnabled" value="1" 
                    {if $mailBoxAutomaticRemovalEnabled} checked {/if} class="center" type='checkbox'>
            {/render}
            <label for="mailBoxAutomaticRemovalValue">{t}Remove mails older than {/t}</label>
            {render acl=$mailBoxAutomaticRemovalACL}
                <input id="mailBoxAutomaticRemovalValue" name="mailBoxAutomaticRemovalValue" 
                    size="6" align="middle" type='text' value="{$mailBoxAutomaticRemovalValue}"  class="center"> {t}days{/t}
            {/render}
        </td>
    </tr>
    {/if}
    {if $localDeliveryOnly_isActive}
    <tr>
        <td>
            {render acl=$localDeliveryOnlyACL}
                <input id='localDeliveryOnly' type=checkbox name="localDeliveryOnly" value="1" 
                    {if $localDeliveryOnly} checked {/if}
                    title="{t}Select if user can only send and receive inside his own domain{/t}" class="center">
            {/render}
            {t}User is only allowed to send and receive local mails{/t}
        </td>
    </tr>
    {/if}
    {if $dropOwnMails_isActive}
    <tr>
        <td>
            {render acl=$dropOwnMailsACL}
                <input id='dropOwnMails' type=checkbox name="dropOwnMails" value="1"    
                    {if $dropOwnMails} checked {/if}
                    title="{t}Select if you want to forward mails without getting own copies of them{/t}">
            {/render}
            {t}No delivery to own mailbox{/t}
        </td>
    </tr>
    {/if}
</table>
{/if}
{/if}
<input type='hidden' name='groupwarePluginPosted' value='1'>
