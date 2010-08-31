<table summary="{t}Mail settings{/t}" style='width:100%;'>
    <tr>
        <td style='width:50%; '>  
            <h3>{t}Generic{/t}</h3>
            <table summary="{t}Mail address configuration{/t}">
                <tr>
                    <td><label for="mailAddress">{t}Primary address{/t}</label>{$must}</td>
                    <td><input type='text' id="mailAddress" name="mailAddress" value="{$mailAddress}"></td>
                </tr>
                <tr>
                    <td><label for="mailLocation">{t}Account location{/t}</label></td>
                    <td>
                        <select size="1" id="mailLocation" name="mailLocation" 
                            title="{t}Specify the location for the mail account{/t}">
                            {html_options values=$mailLocations output=$mailLocations selected=$mailLocation}
                            <option disabled>&nbsp;</option>
                        </select>
                    </td>
                </tr>
                <tr>
                    <td>{t}Quota usage{/t}</td>
                    <td>{$quotaUsage}</td>
                </tr>
                <tr>
                    <td><label for="quotaSize">{t}Quota size{/t}</label></td>
                    <td><input type='text' id="quotaSize" name="quotaSize" value="{$quotaSize}"> MB</td>
                </tr>
                <tr>
                    <td><label for="mailFilter">{t}Mail filter{/t}</label></td>
                    <td><button name='configureFilter'>{t}Configure filter{/t}</button></td>
                </tr>
            </table>

        </td>
        <td class='left-border'>&nbsp;</td>
        <td>
            <h3><label for="alternateAddressList">{t}Alternative addresses{/t}</label></h3>
            <select id="alternateAddressList" style="width:100%;height:100px;" name="alternateAddressList[]" size="15" multiple
                title="{t}List of alternative mail addresses{/t}">
                {html_options values=$alternateAddresses output=$alternateAddresses}
                <option disabled>&nbsp;</option>
            </select>
            <br />
            <input type='text' name="alternateAddressInput">
            <button type='submit' name='addAlternateAddress'>{msgPool type=addButton}</button>
            <button type='submit' name='deleteAlternateAddress'>{msgPool type=delButton}</button>
        </td>
    </tr>
</table>
<hr> 

<table>
    <tr>
        <td style='width:50%'>

            <h3><label for="vacationMessage">{t}Vacation message{/t}</label></h3>

            <table summary="{t}Spam filter configuration{/t}">
                <tr>
                    <td style='width:20px;'>
                        <input type=checkbox name="vacationEnabled" value="1" 
                            {if $vacationEnabled} checked {/if}
                            id="vacationEnabled" 
                            title="{t}Select to automatically response with the vacation message defined below{/t}" 
                            class="center" 
                            onclick="changeState('vacationStart'); changeState('vacationStop'); changeState('vacationMessage');">
                    </td>
                    <td colspan="4">
                        {t}Activate vacation message{/t}
                    </td>
                </tr>
                <tr>
                    <td>&nbsp;</td>
                    <td style='width:30px;'>{t}from{/t}</td>
                    <td>
                        <input type="text" id="vacationStart" name="vacationStart" class="date" 
                            style='width:100px' value="{$vacationStart}"
                            {if !$vacationEnabled} disabled {/if}
                        >
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
                    </td>
                    <td style='width:30px;'>{t}till{/t}</td>
                    <td>
                        <input type="text" id="vacationStop" name="vacationStop" class="date" 
                            style='width:100px' value="{$vacationStop}"i
                            {if !$vacationEnabled} disabled {/if}
                            >
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
                    </td>
                </tr>
                <tr>
                    <td colspan=5>
                        <textarea id="vacationMessage" style="width:99%; height:100px;" 
                            {if !$vacationEnabled} disabled {/if}
                            name="vacationMessage" rows="4" cols="512">{$vacationMessage}</textarea>
                        <br>
                        {if $displayTemplateSelector eq "true"}
                            <select id='vacation_template' name="vacation_template" size=1>
                                {html_options options=$vacationTemplates selected=$vacationTemplate}
                                <option disabled>&nbsp;</option>
                            </select>
                            <button type='submit' name='import_vacation' id="import_vacation">{t}Import{/t}</button>
                        {/if}
                    </td>
                </tr>
            </table>

        </td>
        <td class='left-border'>&nbsp;</td>
        <td>
            <h3><label for="forwardingAddressList">{t}Forward messages to{/t}</label></h3>
            <select id="forwardingAddressList" style="width:100%; height:100px;" name="forwardingAddressList[]" size=15 multiple>
                {html_options values=$forwardingAddresses output=$forwardingAddresses}
                <option disabled>&nbsp;</option>
            </select>
            <br>
            <input type='text' id='forwardingAddressInput' name="forwardingAddressInput">
            <button type='submit' name='addForwardingAddress' id="addForwardingAddress">{msgPool type=addButton}</button>&nbsp;
            <button type='submit' name='addLocalForwardingAddress' id="addLocalForwardingAddress">{t}Add local{/t}</button>&nbsp;
            <button type='submit' name='deleteForwardingAddress' id="deleteForwardingAddress">{msgPool type=delButton}</button>

        </td>
    </tr>
</table>
    
<hr>

<h3>{t}Mailbox options{/t}</h3>
<table summary="{t}Flags{/t}">
    <tr>
        <td>
            <input id='mailBoxWarnLimitEnabled' value='1' name="mailBoxWarnLimitEnabled" value="1" 
                {if $mailBoxWarnLimitEnabled} checked {/if} class="center" type='checkbox'>
            <label for="mailBoxWarnLimitValue">{t}Warn user about a full mailbox when it reaches{/t}</label>
            <input id="mailBoxWarnLimitValue" name="mailBoxWarnLimitValue" 
                size="6" align="middle" type='text' value="{$mailBoxWarnLimitValue}"  class="center"> {t}MB{/t}
        </td>
    </tr>
    <tr>
        <td>
            <input id='mailBoxSendSizelimitEnabled' value='1' name="mailBoxSendSizelimitEnabled" value="1" 
                {if $mailBoxSendSizelimitEnabled} checked {/if} class="center" type='checkbox'>
            <label for="mailBoxSendSizelimitValue">{t}Refuse incoming mails when mailbox size reaches{/t}</label>
            <input id="mailBoxSendSizelimitValue" name="mailBoxSendSizelimitValue" 
                size="6" align="middle" type='text' value="{$mailBoxSendSizelimitValue}"  class="center"> {t}MB{/t}
        </td>
    </tr>
    <tr>
        <td>
            <input id='mailBoxHardSizelimitEnabled' value='1' name="mailBoxHardSizelimitEnabled" value="1" 
                {if $mailBoxHardSizelimitEnabled} checked {/if} class="center" type='checkbox'>
            <label for="mailBoxHardSizelimitValue">{t}Refuse to send and receive mails when mailbox size reaches{/t}</label>
            <input id="mailBoxHardSizelimitValue" name="mailBoxHardSizelimitValue" 
                size="6" align="middle" type='text' value="{$mailBoxHardSizelimitValue}"  class="center"> {t}MB{/t}
        </td>
    </tr>
    <tr>
        <td>
            <input id='mailBoxAutomaticRemovalEnabled' value='1' name="mailBoxAutomaticRemovalEnabled" value="1" 
                {if $mailBoxAutomaticRemovalEnabled} checked {/if} class="center" type='checkbox'>
            <label for="mailBoxAutomaticRemovalValue">{t}Remove mails older than {/t}</label>
            <input id="mailBoxAutomaticRemovalValue" name="mailBoxAutomaticRemovalValue" 
                size="6" align="middle" type='text' value="{$mailBoxAutomaticRemovalValue}"  class="center"> {t}days{/t}
        </td>
    </tr>
    <tr>
        <td>
            <input id='localDeliveryOnly' type=checkbox name="localDeliveryOnly" value="1" 
                {if $localDeliveryOnly} checked {/if}
                title="{t}Select if user can only send and receive inside his own domain{/t}" class="center">
            {t}User is only allowed to send and receive local mails{/t}
        </td>
    </tr>
    <tr>
        <td>
            <input id='dropOwnMails' type=checkbox name="dropOwnMails" value="1"    
                {if $dropOwnMails} checked {/if}
                title="{t}Select if you want to forward mails without getting own copies of them{/t}">
            {t}No delivery to own mailbox{/t}
        </td>
    </tr>
</table>

<input type='hidden' name='groupwarePluginPosted' value='1'>

