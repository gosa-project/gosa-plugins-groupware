        <!-- Avoid posting the first action specified on this page when pressing 'enter' while editing an input field.
                Mostly "dep_root" was posted the base selector was resetted. -->
        <input type='image' src='images/empty.png' name='no_action_posted' value='do_nothing' alt='' style='width:2px;height:2px;'>
	<table summary="" style="width:100%; vertical-align:top; text-align:left;" cellpadding=4>
		<tr>
	 		<td style="vertical-align:top; width:50%;">
			<div class="contentboxh" {if !$is_headpage}  style="height:20px;" {/if}>
				<p class="contentboxh" {if !$is_headpage} style="font-size:12px;" {/if}>
				 	{$Summary} {$hint} 
				</p>
			</div>
  			<div class="contentboxb">
				<div style='background:white;padding:5px;'>
				<table>
					<tr>
						<td>
			      			{$Header}
						</td>
						<td>
      						{$HeaderDropDown}
						</td>
					</tr>
				</table>
				</div>
  			</div>
  			<div style='height:4px;'>
			</div>
  				<div class="contentboxb" style="border-top:1px solid #B0B0B0;">
      			{$DivList}
    			<input type=hidden name="edit_helper">
  			</div>
  		</td>
		{if $is_headpage}
  		<td style="vertical-align:top;">
			{$UserBox1}
   			<div class="contentboxh" style="border-bottom:1px solid #B0B0B0;">
    			<p class="contentboxh">
					<img src="{$infoimage}" align="right" alt="[i]">{t}Information{/t}
				</p>
   			</div>
   			<div class="contentboxb" style="padding:5px;">
				{$Information}
   			</div>
			{$UserBox2}
   			<br>
			{if !$Skip_Filter_Part }
   			<div class="contentboxh">
    			<p class="contentboxh">
					<img src="{$launchimage}" align="right" alt="[F]">{t}Filters{/t}
				</p>
   			</div>
			<div class="contentboxb">
				{if $Display_alphabet}
			 	<table summary="" style="width:100%;border-top:1px solid #B0B0B0;">
			  		{$alphabet}
			 	</table>
				{/if}
				{ if $CheckBoxes != ""}
				<table summary="" style="width:100%;border-top:1px solid #B0B0B0;">
					<tr>
						<td>
							{$CheckBoxes}
						</td>
					</tr>
				</table>   
				{/if}
			{$regexes}
			{$apply}
   			</div>
			{$UserBox3}
			{/if}
  		</td>
		{else}
  		<td style="vertical-align:top;">
			{$UserBox1}
   			<div class="contentboxh" style="border-bottom:1px solid #B0B0B0;height:20px;">
    			<p class="contentboxh">
					<img src="{$infoimage}" align="right" alt="[i]"><font style='font-size:13px;'>{t}Information{/t}</font>
				</p>
   			</div>
   			<div class="contentboxb" style="padding:5px;">
				{$Information}
   			</div>
			{$UserBox2}
   			<br>
			{if !$Skip_Filter_Part }
   			<div class="contentboxh" style="height:20px;">
    			<p class="contentboxh">
					<img src="{$launchimage}" align="right" alt="[F]"><font style='font-size:13px;'>{t}Filters{/t}</font>
				</p>
   			</div>
			<div class="contentboxb">
				{if $Display_alphabet}
			 	<table summary="" style="width:100%;border-top:1px solid #B0B0B0;">
			  		{$alphabet}
			 	</table>
				{/if}
				{ if $CheckBoxes != ""}
				<table summary="" style="width:100%;border-top:1px solid #B0B0B0;">
					<tr>
						<td>
							{$CheckBoxes}
						</td>
					</tr>
				</table>   
				{/if}
			{$regexes}
			{$apply}
			{$UserBox3}
   			</div>
            		{/if}
  		</td>
		{/if}
	</tr>
</table>
{if $Display_Save | $Display_Close}
<p class="seperator">&nbsp;
</p>
<div style="text-align:right;" align="right">
	<p>
		{if $Display_Save}
			<input type="submit" name="SaveMultiSelectWindow" value="{$SaveButtonString}"> 
		{/if}
		{if $Display_Close}
			<input type="submit" name="CloseMultiSelectWindow" value="{$CloseButtonString}"> 
		{/if}
	</p>
</div>
{/if}

<input type="hidden" name="ignore">
<input type="hidden" name="MultiSelectWindow{$filterName}" value="1">
