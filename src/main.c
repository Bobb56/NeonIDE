/*
 * main.c
 *
 *  Created on: Jul 25, 2021
 *      Author: michael
 *      Edited for BOS by Beck
 */

#include "cedit.h"
#include "home.h"
#include "state.h"

bool initialize(struct estate *state)
{
    memset(state->search_buffer,0,255);
	//Default is false, if true, files will be archived after writes. Does nothing on BOS.
	state->autoarchive = false;
	//Default is true, if enabled, unsaved file = prompt
	state->saveprompt = true;
	//Default is true, if enabled, used regular expressions in the search box
	state->useregex = true;
	//Default is true, if enabled, blink the cursor slowly
	state->blinkcursor = true;
	//Default is true, if enabled, write files under a different filename, then remove the existing and rename the new file.
	state->backupfiles = true;
	//Default is true, if enabled, parse ceditrc from /etc/cedit/ceditrc. Otherwise use /home/.ceditrc
	//Does nothin on TIOS
	state->bos_use_system_config = true;
	//Default is false. If enabled, boost BOS maximum buffer size to 128Kb
	//Does nothing on TIOS
	state->bos_use_extra_buffer = false;
	char buf2[10] = "DrMono";
	state->multi_lines = 5;
	state->named = false;
	state->lc1 = 0;
	state->lc2 = MAX_BUFFER_SIZE - 1;
	state->lc_offset = 0;
	state->ls_offset = 0;
	state->c1 = 0;
	state->c2 = MAX_BUFFER_SIZE - 1;
	state->scr_offset = 0;
	state->scr_line_offset = 0;
	state->text_color = 0;
	state->text_highlight_color = 255;
	state->text_selection_color = 120;
	state->text_selection_highlight_color = 30;
	state->background_color = 255;
	state->transparent_color = 1;
	state->statusbar_text_color = 255;
	state->statusbar_color = 4;
	state->border_color = 0;
	state->dropshadow_color = 10;
	state->focus_color = 4;
	strncpy(buf2, state->fontname, 10);
	state->saved = true;

	state->selection_active = false;
	state->alpha_state = 0;

	state->clipboard_size = 0;
	state->corner_radius = 10;

	state->font = 0;
	state->fonttype = 3;
    state->hide_special_files=1;
	//state->text=malloc_noheap(MAX_BUFFER_SIZE);
	//state->text = malloc(MAX_BUFFER_SIZE);
	//Temporary workaround to avoid buffer being yeeted by fileIO.


	state->font = fontlib_GetFontByIndex("DrMono", state->fonttype);
	if (!state->font)
	{
		os_ClrHome();
		os_PutStrFull("E1: Font pack not found.");
		return 1;
	}
	if (!fontlib_SetFont(state->font, 0))
	{
		os_ClrHome();
		os_PutStrFull("E2: Font pack Invalid.");
		return 1;
	}
	fontlib_SetForegroundColor(state->text_color);
	fontlib_SetTransparency(true);
	fontlib_SetBackgroundColor(state->text_highlight_color);

	return 0;
}




int main(void)
{
	gfx_Begin();

	static struct estate editor_state;
	if (initialize(&editor_state)) {
		ngetchx();
		gfx_End();
		exit(0);
	}
	parseRC(&editor_state);
	
	home_menu(&editor_state);
	//launch_editor(&editor_state, NULL);
	
	gfx_End();
	return 0;
}
