/*
 * spaceship.c: A simple spaceship game.
 *
 * St: 2016-10-11 Tue 01:40 PM
 * Up: 2016-10-12 Wed 05:45 AM
 *
 * Author: SPS
 */

#include<stdio.h>
#include<ncurses.h>
#include<assert.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include"mnlib.h"

#define SKIP
#define GROWTH 2

struct position {
	int y;
	int x;
};

struct maximumd {
	int y;
	int x;
};

struct vehicle {
	struct position firep;
	char **body;
};

struct stone {
	struct position pos;
	int alive;
};

struct stonearr {
	struct stone **arr;
	int nmemb;
	int cap;
};

char *sample_body[] = {
				
			  "    .^.    ",
			  "====+ +====",
			  "|   | |   |",
			  "    | |    "
                      };
/* Spaceship */
struct vehicle ship;
#define SHIP_WIDTH  11
#define SHIP_HEIGHT 4 

/* Score */
static int score = 0;

/* Function ptototype declarations */
void del_ship(struct position *p);
void print_ship(struct vehicle *ship, struct position *p);
struct maximumd get_maxd(void);
void print_maxd(struct maximumd *maxd);
void fire(struct vehicle *v);
struct stonearr *create_stonearr(void);
void grow_stonearr(struct stonearr *sta);
void add_stone(struct stonearr *sta, struct stone *newstone);
struct stone *mknew_stone(struct maximumd *maxd);
void run_stones(struct stonearr *sta, struct maximumd *maxd);
void del_stones(struct stonearr *sta);
void print_stones(struct stonearr *sta);
int is_touch(struct stonearr *sta, struct position *pos);
void print_over(struct maximumd *maxd);
void move_ship(struct vehicle *ship, struct position *pos);
void init_score(void);
void update_score(void);
void print_score(struct maximumd *maxd);
int play_again(struct maximumd *maxd);
void init_sta_pos(struct stonearr **sta,
                  struct position *pos, struct maximumd *maxd);
void destroy_stonearr(struct stonearr *sta);
void release_game(struct stonearr *sta);

/* main: start */
int main(void)
{
	struct position pos;
	struct maximumd maxd;
	struct stone *newstone;
	struct stonearr *sta = NULL;

	m_init_scr();

	ship.body = sample_body;
	ship.firep.y = 0;
	ship.firep.x = 0;
	maxd = get_maxd();
	init_sta_pos(&sta, &pos, &maxd);

	init_score();
	while (1) {
		usleep(100000);
		print_ship(&ship, &pos);
		move_ship(&ship, &pos);
		newstone = mknew_stone(&maxd);
		add_stone(sta, newstone);
		del_stones(sta);
		run_stones(sta, &maxd);
		if (is_touch(sta, &pos)) {
			print_over(&maxd);
			if (play_again(&maxd) == 1) {
				init_sta_pos(&sta, &pos, &maxd);
				score = 0;
				clear();
				continue;
			} else {
				release_game(sta);
				break;
			}
		}
		print_stones(sta);
		update_score();
		print_score(&maxd);
		move(maxd.y, 0);
		refresh();
	}

	nocbreak();
	getch();
	m_end_win();

	return 0;
}

/* Initialise stonearr and ship position for game */
void init_sta_pos(struct stonearr **sta,
                  struct position *pos, struct maximumd *maxd)
{
	if (*sta != NULL)
		destroy_stonearr(*sta);
	*sta = create_stonearr();
	pos->y = maxd->y-2;
	pos->x = 10;
}

/* Release memory occupied by game */
void release_game(struct stonearr *sta)
{
	if (sta != NULL)
		destroy_stonearr(sta);
}

/* Destroy stonearr sta */
void destroy_stonearr(struct stonearr *sta)
{
	if (sta == NULL)
		return;

	free(sta->arr);
	free(sta);
}

/* Ask if play again */
int play_again(struct maximumd *maxd)
{
	int c;
	int retval;

	move(maxd->y, 0);
	printw("Game over! Play again? [y/n] : ");
	refresh();
	nocbreak();
	echo();
	c = getch();
	flushinp();
	noecho();
	cbreak();
	move(maxd->y, 0);
	clrtoeol();
	refresh();
	if (c == 'y')
		retval = 1;
	else
		retval = 0;
	return retval;
}

/* Move ship */
void move_ship(struct vehicle *ship, struct position *pos)
{
	int i;
	int c;

	/* timeout(10); */
	nodelay(stdscr, TRUE);;
	i = 0;
	while (i++ < 10 && (c = getch()) != ERR) {
		switch (c) {
		case ('h'):
			del_ship(pos);
			pos->x = pos->x-1;
			print_ship(ship, pos);
			break;
		case 'l':
			del_ship(pos);
			pos->x = pos->x+1;
			print_ship(ship, pos);
			break;
		case 'k':
			del_ship(pos);
			pos->y = pos->y-1;
			print_ship(ship, pos);
			break;
		case 'j':
			del_ship(pos);
			pos->y = pos->y+1;
			print_ship(ship, pos);
			break;
		case 'f':
			ship->firep.y = pos->y-SHIP_HEIGHT;
			ship->firep.x = pos->x+SHIP_WIDTH/2;
			fire(ship);
			break;
		default:
			break;
		}
	}
	flushinp();
	timeout(-1);
}

/* Initialize score to zero */
void init_score(void)
{
	score = 0;
}

/* Update score */
void update_score(void)
{
	score++;
}

/* Print current score */
void print_score(struct maximumd *maxd)
{
	move(0, 0);
	printw("Score: %d", score);
}

/* Print game over message */
void print_over(struct maximumd *maxd)
{
	move(maxd->y, 0);
	printw("Game over !!!");
	refresh();
}

/* Check if any stone is touching ship pos */
int is_touch(struct stonearr *sta, struct position *pos)
{
	int i;
	int nmemb;
	struct position stpos;

	nmemb = sta->nmemb;
	for (i = 0; i < nmemb; i++) {
		if (sta->arr[i] != NULL) {
			stpos = sta->arr[i]->pos;
			if (stpos.y >  pos->y-SHIP_HEIGHT &&
					stpos.y <= pos->y &&
					stpos.x >= pos->x &&
					stpos.x <= pos->x+SHIP_WIDTH)
				return 1;
		}
		    
	}
	return 0;
}

/* Create stone array */
struct stonearr *create_stonearr(void)
{
	struct stonearr *sta;

	sta = malloc(sizeof(struct stonearr));
	assert(sta);
	sta->nmemb = 0;
	sta->arr = calloc(50 , sizeof(struct stone *));
	assert(sta->arr);
	sta->cap = 50;

	return sta;
}

/* Grow a stonearr arr */
void grow_stonearr(struct stonearr *sta)
{
	struct stone **arr;

	arr = realloc(sta->arr, sta->cap * GROWTH * sizeof(struct stone *));
	assert(arr);
	sta->arr = arr;
	sta->cap *= GROWTH;
}

/* Add a stone to stonearr */
void add_stone(struct stonearr *sta, struct stone *newstone)
{
	if (sta->nmemb == sta->cap)
		grow_stonearr(sta);

	sta->arr[sta->nmemb++] = newstone;
}

/* Start stone coming towards ship */
struct stone *mknew_stone(struct maximumd *maxd)
{
	struct stone *newstone;
	static int i = 0;

	newstone = malloc(sizeof(struct stone));
	assert(newstone);
	if (i++ == 0)
		srand(time(NULL));
	newstone->pos.y = 1;
	newstone->pos.x = rand() % (maxd->x+1);
	newstone->alive = 1;

	return newstone;
}

/* Run stones toward ship */
void run_stones(struct stonearr *sta, struct maximumd *maxd)
{
	int i;

	for (i = 0; i < sta->nmemb; i++) {
		if (sta->arr[i] != NULL) {
			if (sta->arr[i]->pos.y == maxd->y) {
				free(sta->arr[i]);
				sta->arr[i] = sta->arr[sta->nmemb-1];
				sta->arr[sta->nmemb-1] = NULL;
				sta->nmemb--;
			} else {
				(sta->arr[i]->pos.y)++;
			}
		}
	}
}

/* Del stones from screen */
void del_stones(struct stonearr *sta)
{
	int i;
	int nmemb;

	nmemb = sta->nmemb;
	for (i = 0; i < nmemb; i++) {
		if (sta->arr[i] != NULL) {
			move(sta->arr[i]->pos.y, sta->arr[i]->pos.x);
			printw(" ");
		}
	}
	refresh();
}

/* Print stones in stonearr to screen */
void print_stones(struct stonearr *sta)
{
	int i;
	int nmemb;

	nmemb = sta->nmemb;
	for (i = 0; i < nmemb; i++) {
		if (sta->arr[i] != NULL) {
			move(sta->arr[i]->pos.y, sta->arr[i]->pos.x);
			printw("*");
		}
	}
	refresh();
}

/* Fire a shot */
void fire(struct vehicle *v)
{
	struct position prev;

	getyx(stdscr, prev.y, prev.x);
	move(v->firep.y, v->firep.x);
	printw("*");
	move(prev.y, prev.x);
	refresh();
}

/*
 * Print ship in a certain position.
 *
 * @ship: Array containing ship
 * @p:    Poisition to print
 */
void print_ship(struct vehicle *ship, struct position *p)
{
	int i;
	struct position prev;

	getyx(stdscr, prev.y, prev.x);
	for (i = 0; i < SHIP_HEIGHT; i++) {
		move(p->y-i, p->x);
		printw("%s", ship->body[SHIP_HEIGHT-i-1]);
	}
	move(prev.y, prev.x);
	refresh();
}

/* Delete the ship which is at the given postition */
void del_ship(struct position *p)
{
	int i;
	int j;
	struct position prev;

	getyx(stdscr, prev.y, prev.x);
	for (i = 0; i < SHIP_HEIGHT; i++) {
		move(p->y-i, p->x);
		for (j = 0; j < SHIP_WIDTH; j++)
			printw(" ");
	}
	move(prev.y, prev.x);
	refresh();
}

/* Get maximum dimensions */
struct maximumd get_maxd(void)
{
	struct maximumd m;
	getmaxyx(stdscr, m.y, m.x);
	m.y--;
	m.x--;
	return m;
}

/* Print maximum dimensions at last line */
void print_maxd(struct maximumd *maxd)
{
	struct position prev;

	getyx(stdscr, prev.y, prev.x);
	move(maxd->y-1, 0);
	printw("Max y: %d, Max x: %d", maxd->y, maxd->x);
	move(prev.y, prev.x);
	refresh();
}

