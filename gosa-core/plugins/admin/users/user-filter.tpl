<div class="contentboxh">
 <p class="contentboxh">
  <img src="images/launch.png" align="right" alt="[F]">{t}Filter{/t}
 </p>
</div>

<div class="contentboxb">

<div style="border-top:1px solid #AAAAAA"></div>

{$TEMPLATES}&nbsp;<LABEL for='TEMPLATES'>{t}Show templates{/t}</LABEL><br>
{$FUNCTIONAL}&nbsp;<LABEL for='FUNCTIONAL'>{t}Show functional users{/t}</LABEL><br>
{$POSIX}&nbsp;<LABEL for='POSIX'>{t}Show POSIX users{/t}</LABEL><br>
{$MAIL}&nbsp;<LABEL for='MAIL'>{t}Show Mail users{/t}</LABEL><br>
{$SAMBA}&nbsp;<LABEL for='SAMBA'>{t}Show Samba users{/t}</LABEL><br>

 <div style="border-top:1px solid #AAAAAA"></div>
 {$SCOPE}

 <table summary="" style="width:100%;border-top:1px solid #B0B0B0;">
  <tr>
   <td>
    <label for="NAME">
     <img src="images/lists/search.png" align=middle>&nbsp;{t}Name{/t}
    </label>
   </td>
   <td>
    {$NAME}
   </td>
  </tr>
 </table>

 <table summary=""  width="100%"  style="background:#EEEEEE;border-top:1px solid #B0B0B0;">
  <tr>
   <td width="100%" align="right">
    {$APPLY}
   </td>
  </tr>
 </table>
</div>
