<h3>{t}Groupware{/t}</h3>

<table summary="{t}Groupware{/t}">
    <tr>
        <td style='width:50%; vertical-align: top;' class='right-border'>
            {t}Mail address{/t}:
            {render acl=$primaryMailAddressACL}
                <input type='text' name="primaryMailAddress" value="{$primaryMailAddress}">
            {/render}
        </td>
        <td style='width:50%; vertical-align: top;'>
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
    </tr>
</table>


