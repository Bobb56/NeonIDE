#include "console.h"
#include "cedit.h"
#include "editor.h"
#include "state.h"
#include "dialogs.h"
#include "tigcclib.h"
#include <string.h>

static int fg_color = 0;

#define INPUT_MAX_LINES		2
#define SEQUENCE_ENTREE     ">> "


void draw_console(struct estate *state)
{
	bool drawn = draw_text_area(state);
	fontlib_SetForegroundColor(state->statusbar_text_color);
	fontlib_SetBackgroundColor(state->text_highlight_color);
	fontlib_SetTransparency(true);
	//Draw statusbars
	gfx_SetColor(state->statusbar_color);
	gfx_FillRectangle_NoClip(0, 0, 320, 12);  //Top
	gfx_FillRectangle_NoClip(1, 228, 62, 12); //Floating segments
	gfx_FillRectangle_NoClip(65, 228, 62, 12);
	gfx_FillRectangle_NoClip(129, 228, 62, 12);
	gfx_FillRectangle_NoClip(193, 228, 62, 12);
	gfx_FillRectangle_NoClip(257, 228, 62, 12);
	//Draw text on segs
	//fontlib_SetCursorPosition(4, 228);
	//fontlib_DrawString("Options");
	fontlib_SetCursorPosition(76, 228);
	fontlib_DrawString("# $ &");
	fontlib_SetCursorPosition(140, 228);
	fontlib_DrawString("Tools");
	fontlib_SetCursorPosition(209, 228);
	fontlib_DrawString("Home");
	//fontlib_SetCursorPosition(276, 228);
	//fontlib_DrawString("Run");
	//Draw drop shadows
	gfx_SetColor(state->dropshadow_color);
	gfx_VertLine_NoClip(63, 229, 11);
	gfx_VertLine_NoClip(127, 229, 11);
	gfx_VertLine_NoClip(191, 229, 11);
	gfx_VertLine_NoClip(255, 229, 11);
	gfx_VertLine_NoClip(319, 229, 11);
	//Top
	gfx_HorizLine_NoClip(0, 12, 320);

	//Draw top text
	fontlib_SetCursorPosition(0, 0);
	fontlib_DrawString("Neon");
	

	fontlib_SetCursorPosition(120, 0);
	if (state->console)
		fontlib_DrawString("Console");
	else
	 	fontlib_DrawString(state->running_program);


	fontlib_SetCursorPosition(280, 0);
	if (state->alpha_state == 1) {
		fontlib_DrawString("alpha");
	}
	else if (state->alpha_state == 2) {
		fontlib_DrawString("ALPHA");
	}

	fontlib_SetForegroundColor(state->text_color);
	fontlib_SetBackgroundColor(state->text_highlight_color);
	fontlib_SetTransparency(true);
	if (!drawn)
	{
		if (state->c1 < state->scr_offset)
		{
			scroll_up(state);
		}
		else
		{
			scroll_down(state);
		}
		gfx_SwapDraw();
		draw_console(state);
	}

	//fontlib_DrawInt(state->selection_anchor, 5);
}

// Removes `nb_lines` lines of text on top of the console buffer
// This operation is invisible to the screen
// Crashes if not enough hidden lines to erase
void console_scroll(struct estate* state, int nb_lines) {
	cursor_to_end(state);

	uint8_t line_number = 0;
	int index = -1;
	uint8_t col_count = 0;
	uint8_t real_line_count = 0; // Counts the number of \n that we will erase
	while (line_number < nb_lines) {
		index++;

		if (state->text[index] == '\n') {
			col_count = 0;
			line_number++;
			real_line_count++;
		}
		else if (col_count == NUM_COLS) {
			col_count = 0;
			line_number++;
		}

		if (state->text[index] != '\n') {
			col_count++;
		}
	}

	state->c1 -= index;
	state->lc1 -= real_line_count;
	state->scr_offset -= index;
	memmove(state->text, state->text + index, state->max_buffer_size - index);
}

void scroll_if_needed(struct estate* state) {
	if (state->c2 - state->c1 < NUM_COLS * INPUT_MAX_LINES || state->lc1 > state->max_lines - INPUT_MAX_LINES) {
		console_scroll(state, NUM_LINES - INPUT_MAX_LINES);
	}
}


void neonide_print_string(struct estate* state, char* text) {
	scroll_if_needed(state);
	cursor_to_end(state);

	while (*text) {
		insert_char(state, *text);
		text++;
	}
	draw_console(state);
	gfx_SwapDraw();
}


void history_next_line(struct estate* state) {

}

void history_prev_line(struct estate* state) {
	
}

void console_cursor_left(struct estate *state, int last_prompt_start)
{
	if (state->selection_active)
	{
		state->selection_active = false;
		if (state->selection_anchor > state->c2 + 1)
			return;
		while (state->selection_anchor < state->c1 - 2)
			cursor_left(state);
	}
	if (state->c1 > last_prompt_start)
	{
		if (state->lc_offset == 0)
		{
			//Moving on same line
			state->lc_offset--;
		}
		state->c1--;
		state->text[state->c2] = state->text[state->c1];
		state->c2--;
	}
}



void console_cursor_left_select(struct estate *state, int last_prompt_start)
{
	if (!state->selection_active)
	{
		state->selection_active = true;
		state->selection_anchor = state->c2;
	}
	if (state->c1 == state->selection_anchor)
	{
		state->selection_anchor = state->c2;
	}
	if (state->c1 > last_prompt_start)
	{
		if (state->lc_offset == 0)
		{
			line_up(state);
		}
		else
		{
			//Moving on same line
			state->lc_offset--;
		}
		state->c1--;
		state->text[state->c2] = state->text[state->c1];
		state->c2--;
	}
}


void console_bs(struct estate *state, int last_prompt_start)
{
	state->saved = false;
	if (state->selection_active)
	{
		state->selection_active = false;
		while (state->selection_anchor < state->c1)
		{ //if selection is to left
			bs(state);
		}
		while (state->selection_anchor > state->c2)
		{ //if selection is to left
			del(state);
		}
	}
	else if (state->c1 > last_prompt_start)
	{
		if (state->lc_offset == 0)
		{
			state->lc1--; //Delete line
			state->lc_offset = state->lines[state->lc1];
			state->lines[state->lc1] += state->lines[state->lc1 + 1];
		}
		else
		{
			state->lc_offset--; //else go back
			state->lines[state->lc1]--;
		}
		state->c1--;
	}
}



char* neonide_input(struct estate *state, char* prompt) {
	neonide_print_string(state, prompt);

    short k = 0;
	int last_prompt_start = state->c1;

    while (true) {
		draw_console(state);
		gfx_SwapDraw();

        k = ngetchx_xy(state, state->cx, state->cy);

        if (!is_control(k))
        {
            if (k == '\n') {
                cursor_to_end(state);

				char* buffer = malloc(state->c1 - last_prompt_start + 1);
				for (int i=0 ; i + last_prompt_start < state->c1 ; i++) {
					buffer[i] = state->text[i + last_prompt_start];
				}
				buffer[state->c1 - last_prompt_start] = '\0';

				insert_char(state, '\n');
				return buffer;
            }
			else {
				insert_char(state, k);
			}
        }
        else
        {
            switch (k) {
                case KEY_CLEAR:
                    return NULL;
				case KEY_SLEFT:
					console_cursor_left_select(state, last_prompt_start);
					break;
				case KEY_SRIGHT:
					cursor_right_select(state);
					break;
                case KEY_LEFT: //left
                    console_cursor_left(state, last_prompt_start);
                    break;
                case KEY_RIGHT: //right
                    cursor_right(state);
                    break;
                case KEY_DOWN: //down
                    history_next_line(state);
                    break;
                case KEY_UP: //up
                    history_prev_line(state);
                    break;
                case KEY_BS: //backspace
                    console_bs(state, last_prompt_start);
                    break;
                case KEY_DEL: //delete
                    del(state);
                    break;
                case KEY_WSLEFT:
                    cursor_to_left_word_select(state);
                    break;
                case KEY_WSRIGHT:
                    cursor_to_right_word_select(state);
                    break;
                case KEY_WSDOWN:
                    cursor_multi_down_select(state);
                    break;
                case KEY_WSUP:
                    cursor_multi_up_select(state);
                    break;
                case KEY_WLEFT: //2nd-left
                    cursor_to_left_word(state);
                    break;
                case KEY_WRIGHT: //2nd-right
                    cursor_to_right_word(state);
                    break;
                case KEY_WUP: //2nd-up
                    cursor_multi_up(state);
                    break;
                case KEY_WDOWN: //2nd-down
                    cursor_multi_down(state);
                    break;
                case KEY_LSLEFT:
                    cursor_to_l_start_select(state);
                    break;
                case KEY_LSRIGHT:
                    cursor_to_l_end_select(state);
                    break;
                case KEY_LSUP:
                    cursor_to_start_select(state);
                    break;
                case KEY_LSDOWN:
                    cursor_to_end_select(state);
                    break;
                case KEY_LLEFT: //meta-left
                    cursor_to_l_start(state);
                    break;
                case KEY_LRIGHT: //meta-right
                    cursor_to_l_end(state);
                    break;
                case KEY_LUP: //meta-up
                    cursor_to_start(state);
                    break;
                case KEY_LDOWN: //meta-down
                    cursor_to_end(state);
                    break;
                case KEY_F1:
                    break;
                case KEY_F2:
					draw_console(state);
					gfx_SwapDraw();
					show_chars_dialog(state, draw_console);
					draw_console(state);
					gfx_SwapDraw();
                    break;
                case KEY_F3:
					draw_console(state);
					gfx_SwapDraw();
					show_console_tools_dialog(state);
					draw_console(state);
					gfx_SwapDraw();
                    break;
                case KEY_F4:
					return NULL;
                    break;
                case KEY_F5:
                    break;
                case KEY_OPEN:
                    break;
                case KEY_STATE:
                    break;
            }
        }
    }
}



void start_using_console(char* name)
{
	struct estate state;
	initialize_console(&state);

	if (name == NULL)
		state.console = true;
	else
	 	strcpy(state.running_program, name);

    fg_color = state.text_color;

    while (true) {
	    char* text = neonide_input(&state, SEQUENCE_ENTREE);
        if (text == NULL)
            return;
		
		if (strlen(text) > 0) {
			neonide_print_string(&state, text);
			neonide_print_string(&state, "\n");
		}
		free(text);
    }

	deinit_state(&state);
}



