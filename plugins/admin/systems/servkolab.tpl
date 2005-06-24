<table summary="" style="width:100%">
 <tr>
  <td style="vertical-align:top; border-right:1px solid #A0A0A0; padding-right:5px;" width="50%">

   <table summary="">
    <tr>
	 <td colspan="2"><b>{t}Services{/t}</b></td>
	</tr>
	<tr>
		<td>
			<input name="cyrus_pop3" value="1" type="checkbox" {$cyrus_pop3Check} {$cyrus_pop3ACL}>
		</td>
		<td>
			{t}POP3 service{/t}
		</td>
	</tr>
	<tr>
        <td>
            <input name="cyrus_pop3s" value="1" type="checkbox" {$cyrus_pop3sCheck} {$cyrus_pop3sACL}>
        </td>
		<td>
			{t}POP3/SSL service{/t}
        </td>
    </tr>
    <tr>
        <td>
            <input name="cyrus_imap" value="1" type="checkbox" {$cyrus_imapCheck} {$cyrus_imapACL}>
        </td>
        <td>
			{t}IMAP service{/t}
        </td>
    </tr>
    <tr>
        <td>
            <input name="cyrus_imaps" value="1" type="checkbox" {$cyrus_imapsCheck} {$cyrus_imapsACL}>
        </td>
        <td>
			{t}IMAP/SSL service{/t}
        </td>
    </tr>
    <tr>
        <td>
            <input name="cyrus_sieve" value="1" type="checkbox" {$cyrus_sieveCheck} {$cyrus_sieveACL}>
        </td>
        <td>
			{t}Sieve service{/t}
        </td>
    </tr>
    <tr>
        <td>
            <input name="proftpd_ftp" value="1" type="checkbox" {$proftpd_ftpCheck} {$proftpd_ftpACL}>
        </td>	
        <td>
			{t}FTP FreeBusy service (legacy, not interoperable with Kolab2 FreeBusy){/t}
        </td>
    </tr>
    <tr>
        <td>
            <input name="apache_http" value="1" type="checkbox" {$apache_httpCheck} {$apache_httpACL}>
        </td>
        <td>
			{t}HTTP FreeBusy service (legacy){/t}
        </td>
    </tr>
    <tr>
        <td>
            <input name="postfix_enable_virus_scan" value="1" type="checkbox" {$postfix_enable_virus_scanCheck} {$postfix_enable_virus_scanACL}>
        </td>
        <td>
			{t}Amavis email scanning (virus/spam){/t}
		</td>
	</tr>
   </table>

   <p class="seperator">&nbsp;</p>
   <br>

   <table summary="">
    <tr>
		<td> 
			<b>{t}Quota settings{/t}</b>
		</td>
	</tr>
	<tr>
		<td>
			{$quotastr}
		</td>
	</tr>
   </table>
  
  </td>
  <td style="vertical-align:top" width="50%">

   <table summary="">
    <tr>
        <td colspan="2">
            <b>{t}Free/Busy settings{/t}</b>
        </td>
    </tr>
    <tr>
        <td>
            <input name="apache_allow_unauthenticated_fb" value="1" type="checkbox" {$apache_allow_unauthenticated_fbCheck} {$apache_allow_unauthenticated_fbACL}> {t}Allow unauthenticated downloading of Free/Busy information{/t}
        </td>
	 </tr>
	 <tr>
        <td>
		{$fbfuture}
	</td>
     </tr>
   </table>

<p class="seperator">&nbsp;</p>
<br>
   <table summary="">
    <tr>
        <td>
        	<b>{t}SMTP privileged networks{/t}</b>
		</td>
    </tr>
    <tr>
	  <td>
	    {t}Hosts/networks allowed to relay{/t}&nbsp;
        <input name="postfix_mynetworks" size="35" maxlength="120" value="{$postfix_mynetworks}" {$postfix_mynetworksACL} type="text">
	</td>
     </tr>
   </table>


<p class="seperator">&nbsp;</p>
<br>


   <table summary="">
    <tr>
        <td>
        	<b>{t}SMTP smarthost/relayhost{/t}</b>
		</td>
    </tr>
    <tr>
        <td>
            <input name="RelayMxSupport" value="1" type="checkbox" {$RelayMxSupportCheck} {$postfix_relayhostACL}>
			{t}Enable MX lookup for relayhost{/t}
		</td>
     </tr>
	<tr>	
		<td>
		    {t}Host used to relay mails{/t}&nbsp;
			<input name="postfix_relayhost" size="35" maxlength="120" value="{$postfix_relayhost}" {$postfix_relayhostACL} type="text">
		</td>
	</tr>
   </table>


<p class="seperator">&nbsp;</p>
<br>


   <table summary="">
    <tr>
        <td>
        	<b>{t}Accept Internet Mail{/t}</b>
		</td>
    </tr>
    <tr>
        <td>
            <input name="postfix_allow_unauthenticated" value="1" type="checkbox" {$postfix_allow_unauthenticatedCheck} {$postfix_allow_unauthenticatedACL}>
        	{t}Accept mail from other domains over non-authenticated SMTP{/t}
		</td>
     </tr>
   </table>


  </td>
 </tr>
</table>

<input type="hidden" name="kolabtab">
