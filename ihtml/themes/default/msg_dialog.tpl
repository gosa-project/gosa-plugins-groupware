{if $frame}
{if $IE}
	<iframe id='e_layer3'
		style="
			position:absolute;
			width:100%;
			height:100%;
			top:0px;
			left:0px;
			border:none;
			border-style:none;
			border-width:0pt;
			display:block;
			allowtransparency='true';
			background-color: #FFFFFF;
			filter:chroma(color=#FFFFFF);
			z-index:0; ">
	</iframe>
	<div  id='e_layer2'
			style='display:none;position:absolute; width:400%;'
<!--
		style="
			position: absolute;
			left: 0px;
			top: 0px;
			right:0px;
			bottom:0px;
			z-index:0;
			width:100%;
			height:100%;
			filter:progid:DXImageTransform.Microsoft.AlphaImageLoader(enabled=true, sizingMethod=scale, src='images/opacity_black.png'); "
-->

{else}
	<div  id='e_layer2'
		style="
			position: absolute;
			left: 0px;
			top: 0px;
			right:0px;
			bottom:0px;
			z-index:0;
			background-image: url(images/opacity_black.png);">

{/if}
{else}


	{if !$JS}

		<div id='e_layer{$i_ID}'
				style='
					width:60%;
					left:200px;
					top:200px;
					background-color:white;
					padding:5px;
					border:5px solid red;
					z-index:150;
					position:absolute;'>
			<table style='width:100%' summary='' border=0>
				<tr>
					<td style='vertical-align:top;padding:10px'>
	{if $i_Type == ERROR_DIALOG}
						<img src='images/error.png' alt='{t}Error{/t}'>
	{elseif $i_Type == WARNING_DIALOG}
						<img src='images/warning.png'  alt='{t}Warning{/t}'>
	{elseif $i_Type == INFO_DIALOG || $i_Type == CONFIRM_DIALOG}
						<img src='images/info.png' alt='{t}Information{/t}'>
	{/if}
					</td>
					<td style='width:100%'>
						<h1>{$s_Title}</h1>
						<b>{$s_Message}</b>
						<br>
						<br>
					</td>
				</tr>
				<tr>
					<td colspan='2' align='center'>
	{if $i_Type == ERROR_DIALOG || $i_Type == WARNING_DIALOG || $i_Type == INFO_DIALOG}
						<button type='submit' name='MSG_OK{$i_ID}'>{t}Ok{/t}</button>
	{elseif $i_Type == CONFIRM_DIALOG}
						<button type='submit' name='MSG_OK{$i_ID}'>{t}Ok{/t}</button>
						<button type='submit' name='MSG_CANCEL{$i_ID}'>{t}Cancel{/t}</button>
	{/if}
					</td>
				</tr>
			</table>
		</div>

	{else}

		{if $s_Trace != "" && $i_TraceCnt != 0}
		<div id='trace_{$i_ID}' style='visibility:hidden;'>
			{$s_Trace}
		</div>
		
		{/if}

		<div id='e_layer{$i_ID}'
				style='
					width:60%;
					left:200px;
					top:200px;
					background-color:white;
					padding:5px;
					border:5px solid red;
					z-index:150;
					display:none;
					position:absolute;'>
			<table style='width:100%' summary='' border=0>
				<tr>
					<td style='vertical-align:top;padding:10px'>
	{if $i_Type == ERROR_DIALOG}
						<img src='images/error.png' alt='{t}Error{/t}'>
	{elseif $i_Type == WARNING_DIALOG}
						<img src='images/warning.png'  alt='{t}Warning{/t}'>
	{elseif $i_Type == INFO_DIALOG || $i_Type == CONFIRM_DIALOG}
						<img src='images/info.png' alt='{t}Information{/t}'>
	{/if}
					</td>
					<td style='width:100%'>
						<h1>{$s_Title}</h1>
						<b>{$s_Message}</b>
						<br>
						<br>
					</td>
					{if $s_Trace != "" && $i_TraceCnt != 0}
					<td style='width:20px; vertical-align:top; cursor:pointer;'>
						<div onClick="toggle('trace_{$i_ID}')"><u>Trace</u></div>
					</td>
					{/if}
				</tr>
				<tr>
					<td colspan='2' align='center'>
	{if $i_Type == ERROR_DIALOG || $i_Type == WARNING_DIALOG || $i_Type == INFO_DIALOG}
						<button type='button' name='MSG_OK{$i_ID}' onClick='next_msg_dialog();'>{t}Ok{/t}</button>
	{elseif $i_Type == CONFIRM_DIALOG}
						<button type='submit' name='MSG_OK{$i_ID}' onClick='next_msg_dialog();'>{t}Ok{/t}</button>
						<button type='button' name='MSG_CANCEL{$i_ID}' onClick='next_msg_dialog();'>{t}Cancel{/t}</button>
	{/if}
					</td>
				</tr>
			</table>
		</div>
	{/if}
{/if}
