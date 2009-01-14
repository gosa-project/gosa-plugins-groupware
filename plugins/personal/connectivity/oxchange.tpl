{if !$pg}
<h2>{t}Open-Xchange Account{/t} - {t}disabled, no Postgresql support detected. Or the specified database can't be reached{/t}</h2>
{else}

<input type="checkbox" name="oxchange" value="B" 
	{$oxchangeState} {$oxchangeAccountACL} 
	onCLick="	changeState('OXAppointmentDays');
				changeState('OXTaskDays');
				changeState('OXTimeZone');" >
<h2>{t}Open-Xchange account{/t}</h2>


<table summary="" style="width:100%; vertical-align:top; text-align:left;" cellpadding=0 border=0>

 <!-- Headline container -->
 <tr>
   <td style="width:50%; vertical-align:top;">
     <table summary="" style="margin-left:4px;">
       <tr>
         <td colspan=2 style="vertical-align:top;">
           <b>{t}Remember{/t}</b>
         </td>
       </tr>
       <tr>
         <td><LABEL for="OXAppointmentDays">{t}Appointment Days{/t}</LABEL></td>
	 <td><input name="OXAppointmentDays" id="OXAppointmentDays" size=7 maxlength=7 {$OXAppointmentDaysACL} value="{$OXAppointmentDays}" {$oxState}> {t}days{/t}</td>
       </tr>
       <tr>
         <td><LABEL for="OXTaskDays">{t}Task Days{/t}</LABEL></td>
	 <td><input name="OXTaskDays" id="OXTaskDays" size=7 maxlength=7 {$OXTaskDaysACL} value="{$OXTaskDays}" {$oxState}> {t}days{/t}</td>
       </tr>
     </table>
   </td>
   <td rowspan=2 style="border-left:1px solid #A0A0A0">
     &nbsp;
   </td>
   <td style="vertical-align:top;">
     <table summary="">
       <tr>
         <td colspan=2 style="vertical-align:top;">
           <b>{t}User Information{/t}</b>
         </td>
       </tr>
       <tr>
         <td><LABEL for="OXTimeZone">{t}User Timezone{/t}</LABEL></td>
	 <td><select size="1" name="OXTimeZone" id="OXTimeZone" {$OXTimeZoneACL} {$oxState}> 
	 {html_options values=$timezones output=$timezones selected=$OXTimeZone}
	 </select>
	 </td>
       </tr>
       <tr>
         <td></td>
	 <td></td>
       </tr>
     </table>
   </td>
 </tr>
</table>
{/if}