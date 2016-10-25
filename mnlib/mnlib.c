/* mnlib.c */

/* source code for mnlib */

/* St: 2016-03-31 Thu 12:31 AM */
/* UP: 2016-03-31 Thu 12:43 AM */

#include<ncurses.h>
#include<assert.h>

/* mInitScr: initialize screen */
void m_init_scr(void)
{
	WINDOW *win;

	win = initscr();
	assert(win != NULL);
	start_color();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	init_pair(1, COLOR_GREEN, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_BLACK);
}

/* mEndWin: end ncurses session */
void m_end_win(void)
{
	endwin();
}

