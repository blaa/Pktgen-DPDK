/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2016-2017 Intel Corporation. All rights reserved.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _CLI_INPUT_H_
#define _CLI_INPUT_H_

/**
 * @file
 * RTE Command line input interface
 *
 */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Poll for a input character
 *
 * @param c
 *   Pointer to character address.
 * @return
 *   return non-zero if character was read.
 */
int cli_poll(char *c);

/**
 * The CLI write routine, using write() call
 *
 * @note Uses thread variable this_cli.
 *
 * @param msg
 *   The string to be written
 * @param len
 *   Number of bytes to write or if -1 then strlen(msg) is used.
 * @return
 *   Number of bytes written.
 */
int cli_write(const void *msg, int len);

/**
 * The routine to read characters from the user input.
 *
 * @param buf
 *   The string buffer to put the read characters.
 * @param len
 *   The length of the <buf> array to put the input.
 * @return
 *   The number of bytes read.
 */
int cli_read(char *buf, int len);

/* Report Cursor Position	<ESC>[{ROW};{COLUMN}R
   Generated by the device in response to a Query Cursor Position request;
   reports current cursor position. */
static inline void
cli_get_cursor(int *row, int *col)
{
	char buf[32], *p, ch;
	int r, c, l;

again:
	scrn_cpos();

	memset(buf, 0, sizeof(buf));
	p = buf;
	l = sizeof(buf) - 1;

	do {
		cli_read(&ch, 1);
		if (ch == 'R')
			break;
		if (ch == '\0')
			continue;
		*p++ = ch;
	} while(l--);

	p = index(buf, ';');
	if (!p)
		goto again;

	r = atoi(&buf[2]);
	c = atoi(++p);
	if (!r || !c)
		goto again;

	*row = r;
	*col = c;
}

/**
 * Move the vt100 cursor to the left one character
 *
 * @note Uses thread variable this_cli.
 *
 * @return
 *   N/A
 */
static inline void
cli_cursor_left(void)
{
	cli_write(vt100_left_arr, -1);
}

/**
 * Move the vt100 cursor to the right one character
 *
 * @note Uses thread variable this_cli.
 *
 * @return
 *   N/A
 */
static inline void
cli_cursor_right(void)
{
	cli_write(vt100_right_arr, -1);
}

/**
 * Save the vt100 cursor location
 *
 * @note Uses thread variable this_cli.
 *
 * @return
 *   N/A
 */
static inline void
cli_save_cursor(void)
{
	cli_write(vt100_save_cursor, -1);
}

/**
 * Restore the cursor to the saved location on the console
 *
 * @note Uses thread variable this_cli.
 *
 * @return
 *   N/A
 */
static inline void
cli_restore_cursor(void)
{
	cli_write(vt100_restore_cursor, -1);
}

/**
 * Print out the left side of the input in the Gap Buffer.
 *
 * @note Uses thread variable this_cli.
 *
 * @return
 *   N/A
 */
static inline void
cli_display_left(void)
{
	if (gb_left_data_size(this_cli->gb))
		cli_write(gb_start_of_buf(this_cli->gb),
		          gb_left_data_size(this_cli->gb));
}

/**
 * Print out the right side of the input in the Gap Buffer.
 *
 * @note Uses thread variable this_cli.
 *
 * @return
 *   N/A
 */
static inline void
cli_display_right(void)
{
	if (gb_right_data_size(this_cli->gb))
		cli_write(gb_end_of_gap(this_cli->gb),
		          gb_right_data_size(this_cli->gb));
}

/**
 * Clear the console screen
 *
 * @note Uses thread variable this_cli.
 *
 * @return
 *   N/A
 */
static inline void
cli_clear_screen(void)
{
	cli_write(vt100_clear_screen, -1);
}

/**
 * clear from cursor to end of line
 *
 * @note Uses thread variable this_cli.
 *
 * @return
 *   N/A
 */
static inline void
cli_clear_to_eol(void)
{
	cli_write(vt100_clear_right, -1);
}

/**
 * Clear the current line or the line given
 *
 * @note Uses thread variable this_cli.
 *
 * @param lineno
 *   if lineno is -1 then clear the current line else the lineno given.
 * @return
 *   N/A
 */
static inline void
cli_clear_line(int lineno)
{
	if (lineno > 0)
		cli_printf(vt100_pos_cursor, lineno, 0);
	else
		cli_write("\r", 1);

	cli_write(vt100_clear_line, -1);
}

/**
 * Move the cursor up by the number of lines given
 *
 * @note Uses thread variable this_cli.
 *
 * @param lineno
 *   Number of lines to move the cursor
 * @return
 *   N/A
 */
static inline void
cli_move_cursor_up(int lineno)
{
	while (lineno--)
		cli_printf(vt100_up_arr);
}

static inline void
cli_display_prompt(int t)
{
	cli_write("\r", 1);
	this_cli->plen = this_cli->prompt(t);
	cli_clear_to_eol();
}

/* display all or part of the command line, while allowing the line to scroll */
static inline void
cli_display_line(void)
{
	struct gapbuf *gb = this_cli->gb;
	char buf[gb_data_size(gb) + 16];
	int point = gb_point_offset(gb);
	int len = gb_copy_to_buf(gb, buf, gb_data_size(gb));
	int window = (this_scrn->ncols - this_cli->plen) - 1;
	int wstart, wend;

	if (cli_tst_flag(DELETE_CHAR)) {
		cli_clr_flag(DELETE_CHAR);
		cli_write(" \b", 2);
		cli_set_flag(DISPLAY_LINE | CLEAR_TO_EOL);
	}
	if (cli_tst_flag(CLEAR_LINE)) {
		cli_clr_flag(CLEAR_LINE);
		scrn_bol();
		cli_clear_to_eol();
		cli_set_flag(DISPLAY_LINE | DISPLAY_PROMPT);
	}
	if (cli_tst_flag(CLEAR_TO_EOL)) {
		cli_clr_flag(CLEAR_TO_EOL);
		cli_clear_to_eol();
	}
	if (cli_tst_flag(DISPLAY_PROMPT)) {
		cli_clr_flag(DISPLAY_PROMPT | PROMPT_CONTINUE);
		cli_display_prompt(0);
	}

	if (point < window) {
		wstart = 0;
		if (len < window)
			wend = point + (len - point);
		else
			wend = point + (window - point);
	} else {
		wstart = point - window;
		wend = wstart + window;
	}

	scrn_bol();
	scrn_cnright(this_cli->plen);

	cli_write(&buf[wstart], wend - wstart);

	cli_clear_to_eol();

	scrn_bol();
	scrn_cnright(this_cli->plen + point);
}

/**
 * Print out the complete line in the Gap Buffer.
 *
 * @note Uses thread variable this_cli.
 *
 * @return
 *   N/A
 */
static inline void
cli_redisplay_line(void)
{
	uint32_t i;

	this_cli->flags |= DISPLAY_PROMPT;

	cli_display_line();

	gb_move_gap_to_point(this_cli->gb);

	for (i = 0;
	     i < (gb_data_size(this_cli->gb) - gb_point_offset(this_cli->gb));
	     i++)
		cli_cursor_left();
}

/**
 * Add a input text string the cli input parser
 *
 * @note Uses thread variable this_cli.
 *
 * @param str
 *   Pointer to string to insert
 * @param n
 *   Number of bytes in string
 * @return
 *   N/A
 */
void cli_input(char *str, int n);

/**
 * Set the CLI prompt function pointer
 *
 * @param prompt
 *   Function pointer to display the prompt
 * @return
 *   Return the old prompt function pointer or NULL if one does not exist
 */
cli_prompt_t cli_set_prompt(cli_prompt_t prompt);

/**
 * Set the I/O file descriptors
 *
 * @note Uses thread variable this_cli.
 *
 * @param in
 *   File descriptor for input
 * @param out
 *   File descriptor for output
 * @return
 *   N/A
 */
void cli_set_io(FILE *in, FILE *out);

/**
 * Set the I/O to use stdin/stdout
 *
 * @note Uses thread variable this_cli.
 *
 * @return
 *   0 on success or non-0 on error
 */
int cli_stdin_setup(void);

/**
 * Restore the stdin/stdout tty params from setup
 *
 * @note Uses thread variable this_cli.
 *
 * @return
 *   N/A
 */
void cli_stdin_restore(void);

/**
 * Pause and wait for input character
 *
 * @note Uses thread variable this_cli.
 *
 * @param keys
 *   List of keys to force return, if NULL defaults to ESC and q/Q
 * @return
 *   character that terminated the pause or zero.
 */
char cli_pause(const char *msg, const char *keys);

/**
 * return true if calling yield should are enabled.
 *
 * @note Uses thread variable this_cli.
 *
 * @return
 *   non-zero if true else 0
 */
int cli_yield_io(void);

#ifdef __cplusplus
}
#endif

#endif /* _CLI_INPUT_H_ */