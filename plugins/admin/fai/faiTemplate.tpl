<table summary="" width="100%">
	<tr>
		<td valign="top" style="border-right:1px solid #A0A0A0; width:50%">
				<h2><img class="center" alt="" src="images/fai_small.png" align="middle" title="{t}Generic{/t}">&nbsp;{t}Generic{/t}</h2>
				<table summary="" cellspacing="4">
					<tr>
						<td>
							<LABEL for="cn">
							{t}Name{/t}{$must}
							</LABEL>
						</td>
						<td>
							<input value="{$cn}" size="45" maxlength="80" disabled id="cn">
						</td>
					</tr>
					<tr>
						<td>
							<LABEL for="description">
							{t}Description{/t}
							</LABEL>
						</td>
						<td>
							<input value="{$description}" size="45" maxlength="80" {$description} name="description" id="description" {$descriptionACL}>
						</td>
					</tr>
				</table>
		</td>
		<td style="width:50%">
				<h2><img class="center" alt="" src="images/fai_template.png" align="middle" title="{t}Objects{/t}">&nbsp;
					<LABEL for="SubObject">
						{t}List of template files{/t}
					</LABEL>
				</h2>
			{$Entry_divlist}
				<input type="submit" name="AddSubObject"     value="{t}Add{/t}"		title="{t}Add{/t}" {$cnACL}>
		</td>
	</tr>
</table>
<input type="hidden" value="1" name="FAItemplate_posted">
<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('cn','description');
  -->
</script>
