<input type="hidden" name="SubObjectFormSubmitted" value="1">
<table width="100%" summary="">
	<tr>
		<td valign="top" width="50%">
			<h2><img class="center" alt="" src="images/fai_small.png" align="middle" title="{t}Generic{/t}">&nbsp;{t}Generic{/t}</h2>
				<table summary="">
					<tr>
						<td>
							{t}Name{/t}{$must}&nbsp;
						</td>
						<td>
							<input value="{$cn}" size="45" maxlength="80" name="cn" {$cnACL}>
						</td>
					</tr>
					<tr>
						<td>
							{t}Description{/t}&nbsp;
						</td>
						<td>
							<input value="{$description}" size="45" maxlength="80" name="description" {$descriptionACL}>
						</td>
					</tr>
				</table>
		</td>
		<td style="border-left: 1px solid rgb(160, 160, 160);">
           &nbsp;
        </td>
		<td valign="top">
				<h2><img class="center" alt="" src="images/fai_variable.png" align="middle" title="{t}Variable attributes{/t}">&nbsp;{t}Variable attributes{/t}</h2>
				<table  summary="" width="100%">
					<tr>
						<td>
							<LABEL for="Content">
							{t}Variable content{/t}{$must}&nbsp;
							</LABEL>
						</td>
						<td>
							<input type="text" name="FAIvariableContent" value="{$FAIvariableContent}" id="Content" style="width:250px;" {$FAIvariableContentACL}>
						</td>
					</tr>
				</table>
		</td>
	</tr>
</table>
<p class="seperator">&nbsp;</p>
<br>
<div style="align:right;" align="right">
	<input type="submit" value="{t}Apply{/t}" 	name="SaveSubObject" {$cnACL}>&nbsp;
	<input type="submit" value="{t}Cancel{/t}" 	name="CancelSubObject">
</div>

<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('cn','description');
  -->
</script>
