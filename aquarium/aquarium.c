/*
 * funwindow.c: One another silly game implemented in C.
 *
 * St: 2016-10-13 Thu 04:47 AM
 * Up: 2016-10-13 Thu 07:29 AM
 *
 * Author: SPS
 */

#include<stdio.h>
#include<ncurses.h>
#include<string.h>
#include<assert.h>
#include"mnlib.h"

#define SKIP

#define PLAYW_HEIGHT   20
#define PLAYW_WIDTH    20
#define PLAYW_STARTY   10
#define PLAYW_STARTX   10

#define SCOREW_HEIGHT   5
#define SCOREW_WIDTH    20
#define SCOREW_STARTY   40
#define SCOREW_STARTX   10

#define LEFT   1
#define RIGHT  2
#define UP     3
#define DOWN   4
#define ILLDIR 5

#define MOVE   10
#define CDIR   11
#define ILLOPR 12

struct position {
	int y;
	int x;
};

struct stick {
	struct position pos;
	int dir;
	char str[3];
};

/* Dimensions of windows */
int playw_height;
int playw_width;
int playw_starty;
int playw_startx;

int scorew_height;
int scorew_width;
int scorew_starty;
int scorew_startx;

/* Score */
int score = 0;

/* Function prototype declarations */
WINDOW *create_playwin(int height, int width, int starty, int startx);
WINDOW *create_scorewin(int height, int width, int starty, int startx);
WINDOW *create_newwin(int height, int width, int starty, int startx);
void boxify(WINDOW *win);
void init_stick(struct stick *jim);
int  move_stick(struct stick *jim, int c);
void del_stick(WINDOW *playwin, struct stick *jim);
void print_stick(WINDOW *playwin, struct stick *jim);
void init_windows(WINDOW **playwin, WINDOW **scorewin);
void update_score(void);
void print_score(WINDOW *swin, int score);
int get_opr(int c);
int rot_stick(struct stick *jim, int c);

/* main: start */
int main(void)
{
	WINDOW *playwin;
	WINDOW *scorewin;
	struct stick jim;
	struct stick oldjim;
	int c;
	int opr;

	m_init_scr();
	refresh();

	init_windows(&playwin, &scorewin);
	init_stick(&jim);

	print_stick(playwin, &jim);
	print_score(scorewin, score);
	move(0, 0);
	refresh();

	while (1) {
		oldjim = jim;
		c = getch();
		opr = get_opr(c);
		if (opr == MOVE && move_stick(&jim, c) == 1) {
			update_score();
			print_score(scorewin, score);
			del_stick(playwin, &oldjim);
			print_stick(playwin, &jim);
		} else if (opr == CDIR && rot_stick(&jim, c) == 1) {
			del_stick(playwin, &oldjim);
			print_stick(playwin, &jim);
		}
		move(0, 0);
		refresh();
	}

	getch();
	m_end_win();
	return 0;
}
/* Rotate stick */ int rot_stick(struct stick *jim, int c)
{
	int cdir;

	if (c == 'H')
		cdir = LEFT;
	else if (c == 'L')
		cdir = RIGHT;
	else
		cdir = ILLDIR;

	if (jim->dir == cdir)
		return 0;

	jim->dir = cdir;
	if (cdir == LEFT)
		strncpy(jim->str, "<-", 3);
	else if (cdir == RIGHT)
		strncpy(jim->str, "->", 3);

	return 1;
}

/* Get operation idicated by user */
int get_opr(int c)
{
	switch(c) {
	case 'H':
	case 'L':
		return CDIR;
		break;
	case 'h':
	case 'j':
	case 'k':
	case 'l':
		return MOVE;
		break;
	default:
		return ILLOPR;
		break;
	}
}

/* Initialise play and score windows */
void init_windows(WINDOW **playwin, WINDOW **scorewin)
{
	playw_height = 10;
	playw_width = 30;
	playw_starty = (LINES - playw_height) / 2;
	playw_startx = (COLS - playw_width) / 2;

	scorew_height = 5;
	scorew_width = 20;
	scorew_starty = playw_starty + playw_height;
	scorew_startx = (COLS - scorew_width) / 2;

	*playwin = create_playwin(playw_height, playw_width,
	                         playw_starty, playw_startx);
	boxify(*playwin);
	wrefresh(*playwin);

	*scorewin = create_scorewin(scorew_height, scorew_width,
	                           scorew_starty, scorew_startx);
	boxify(*scorewin);
	wrefresh(*scorewin);
}

/* Put a box around window */
void boxify(WINDOW *win)
{
	box(win, 0, 0);
}

/* Create play window */
WINDOW *create_playwin(int height, int width, int starty, int startx)
{
	WINDOW *playwin;

	playwin = create_newwin(height, width, starty, startx);

	return playwin;
}

/* Create score window */
WINDOW *create_scorewin(int height, int width, int starty, int startx)
{
	WINDOW *scorewin;

	scorewin = create_newwin(height, width, starty, startx);

	return scorewin;
}

/* Create a new window */
WINDOW *create_newwin(int height, int width, int starty, int startx)
{
	WINDOW *win;

	win = newwin(height, width, starty, startx);
	assert(win);

	return win;
}

/* Initialize stick */
void init_stick(struct stick *jim)
{
	jim->pos.y = playw_height - 1 - 1;
	jim->pos.x = 1;
	jim->dir = RIGHT;
	strncpy(jim->str, "->", 3);
}

/* Move a stick */
int move_stick(struct stick *jim, int c)
{
	int movedir;

	switch (c) {
	case 'h':
		movedir = LEFT;
		break;
	case 'j':
		movedir = DOWN;
		break;
	case 'k':
		movedir = UP;
		break;
	case 'l':
		movedir = RIGHT;
		break;
	default:
		return 0;
	}

	if ((movedir == LEFT || movedir == RIGHT) && (movedir != jim->dir))
		return 0;
	
	if (movedir == LEFT && jim->pos.x > 1) {
		jim->pos.x--;
		return 1;
	} else if (movedir == RIGHT && jim->pos.x < playw_width-3) {
		jim->pos.x++;
		return 1;
	} else if (movedir == UP && jim->pos.y > 1) {
		jim->pos.y--;
		return 1;
	}else if (movedir == DOWN && jim->pos.y < playw_height-2) {
		jim->pos.y++;
		return 1;
	}

	return 0;
}

/* Del stick in playwindow */
void del_stick(WINDOW *playwin, struct stick *jim)
{
	int y;
	int x;

	y = jim->pos.y;
	x = jim->pos.x;
	
	mvwprintw(playwin, y, x, "  ");

	wrefresh(playwin);
}

/* Print stick in playwindow */
void print_stick(WINDOW *playwin, struct stick *jim)
{
	int y;
	int x;

	y = jim->pos.y;
	x = jim->pos.x;
	
	mvwprintw(playwin, y, x, "%s", jim->str);

	wrefresh(playwin);
}

/* Update score */
void update_score(void)
{
	score++;
}

/* Print score in score window */
void print_score(WINDOW *swin, int score)
{
	mvwprintw(swin, 1, 1, "Score: %d", score);
	wrefresh(swin);
}

