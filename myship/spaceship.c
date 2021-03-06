/*
 * spaceship.c: A simple spaceship game.
 *
 * St: 2016-10-11 Tue 01:40 PM
 * Up: 2016-10-13 Thu 04:26 AM
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
};

struct fire {
	struct position pos;
};

struct pixinfo {
	int sto;
	int fir;
};

struct screen {
	struct pixinfo **info;
};

struct stonearr {
	struct stone **arr;
	int nmemb;
	int cap;
};

struct firearr {
	struct fire **arr;
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

/* Screen info */
struct screen scr_info;

/* Color info */
int color = 0;

/* Function ptototype declarations */
void del_ship(struct position *p);
void print_ship(struct vehicle *ship, struct position *p);
struct maximumd get_maxd(void);
void print_maxd(struct maximumd *maxd);
void fire(struct firearr *fira, struct vehicle *v);
struct stonearr *create_stonearr(void);
void grow_stonearr(struct stonearr *sta);
void add_stone(struct stonearr *sta, struct stone *newstone);
struct stone *mknew_stone(struct maximumd *maxd);
void run_stones(struct stonearr *sta, struct maximumd *maxd);
void del_stones(struct stonearr *sta);
void print_stones(struct stonearr *sta);
int is_touch(struct stonearr *sta, struct position *pos);
void print_over(struct maximumd *maxd);
void move_ship(struct vehicle *ship,
               struct position *pos, struct firearr *fira);
void init_score(void);
void update_score(void);
void print_score(struct maximumd *maxd);
int play_again(struct maximumd *maxd);
void init_game(struct stonearr **sta, struct firearr **fira,
                     struct position *pos, struct maximumd *maxd);
void destroy_stonearr(struct stonearr *sta);
void release_game(struct stonearr *sta, struct firearr *fira);
void init_scr_info(struct screen *scr_info, struct maximumd *maxd);
void release_scr_info(struct screen *scr_info, struct maximumd *maxd);
struct firearr *create_firearr(void);
struct fire *mknew_fire(struct position *pos);
void grow_firearr(struct firearr *fira);
void add_fire(struct firearr *fira, struct fire *newfire);
void run_fires(struct firearr *fira);
void del_fires(struct firearr *fira);
void print_fires(struct firearr *fira);
void destroy_firearr(struct firearr *fira);
int init_color_pairs(void);
void burn_ship(struct vehicle *ship, struct position *pos);

/* main: start */
int main(void)
{
	struct position pos;
	struct maximumd maxd;
	struct stone *newstone;
	struct stonearr *sta = NULL;
	struct firearr *fira = NULL;

	m_init_scr();

	ship.body = sample_body;
	ship.firep.y = 0;
	ship.firep.x = 0;
	maxd = get_maxd();
	init_game(&sta, &fira, &pos, &maxd);
	init_scr_info(&scr_info, &maxd);
	color = init_color_pairs();

	init_score();
	while (1) {
		usleep(70000);
		print_ship(&ship, &pos);
		move_ship(&ship, &pos, fira);
		del_fires(fira);
		run_fires(fira);
		print_fires(fira);
		newstone = mknew_stone(&maxd);
		add_stone(sta, newstone);
		del_stones(sta);
		run_stones(sta, &maxd);
		if (is_touch(sta, &pos)) {
			burn_ship(&ship, &pos);
			del_fires(fira);
			print_over(&maxd);
			if (play_again(&maxd) == 1) {
				init_game(&sta, &fira, &pos, &maxd);
				release_scr_info(&scr_info, &maxd);
				init_scr_info(&scr_info, &maxd);
				score = 0;
				clear();
				continue;
			} else {
				release_game(sta, fira);
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
	release_scr_info(&scr_info, &maxd); 
	m_end_win();

	return 0;
}

/* Burn the ship in screen -> print a burning ship */
void burn_ship(struct vehicle *ship, struct position *pos)
{
	int i;
	struct position prev;

	getyx(stdscr, prev.y, prev.x);
	if (color == 1) {
		attron(COLOR_PAIR(1));
		attron(A_STANDOUT);
	}
	for (i = 0; i < SHIP_HEIGHT; i++) {
		move(pos->y-i, pos->x);
		printw("%s", ship->body[SHIP_HEIGHT-i-1]);
	}
	if (color == 1) {
		attroff(COLOR_PAIR(1));
		attroff(A_STANDOUT);
	}
	move(prev.y, prev.x);
	refresh();
}

/* Initialize color pairs */
int init_color_pairs(void)
{
	if (has_colors() == FALSE)
		return 0;
	start_color();
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_BLUE, COLOR_BLACK);
	return 1;
}

/* Create firearr fira */
struct firearr *create_firearr(void)
{
	struct  firearr *fira;

	fira = malloc(sizeof(struct firearr));
	assert(fira);
	fira->nmemb = 0;
	fira->arr = calloc(50 , sizeof(struct fire *));
	assert(fira->arr);
	fira->cap = 50;

	return fira;
}

/* Run firearr fira */
void run_fires(struct firearr *fira)
{
	int i;
	struct fire *f;

	for (i = 0; i < fira->nmemb; i++) {
		if ((f = fira->arr[i]) != NULL) {
			if (f->pos.y == 1) {
				scr_info.info[f->pos.y][f->pos.x].fir = 0; 
				free(f);
				fira->arr[i] = fira->arr[fira->nmemb-1];
				fira->arr[fira->nmemb-1] = NULL;
				fira->nmemb--;
			} else {
				scr_info.info[f->pos.y][f->pos.x].fir = 0; 
				(f->pos.y)--;
				scr_info.info[f->pos.y][f->pos.x].fir = 1; 
			}
		}
	}
}

/* Del fires from screen */
void del_fires(struct firearr *fira)
{
	int i;
	int nmemb;

	nmemb = fira->nmemb;
	for (i = 0; i < nmemb; i++) {
		if (fira->arr[i] != NULL) {
			move(fira->arr[i]->pos.y, fira->arr[i]->pos.x);
			printw(" ");
		}
	}
	refresh();
}

/* Print fires in firearr to screen */
void print_fires(struct firearr *fira)
{
	int i;
	int nmemb;

	nmemb = fira->nmemb;
	for (i = 0; i < nmemb; i++) {
		if (fira->arr[i] != NULL) {
			move(fira->arr[i]->pos.y, fira->arr[i]->pos.x);
			if (color == 1) {
				attron(COLOR_PAIR(2));
				attron(A_BOLD);
			}
			printw("^");
			if (color == 1) {
				attroff(COLOR_PAIR(2));
				attroff(A_BOLD);
			}
		}
	}
	refresh();
}

/* Initialize screen info */
void init_scr_info(struct screen *scr_info, struct maximumd *maxd)
{
	int i;

	scr_info->info = malloc((maxd->y+1) * sizeof(struct pixinfo *));
	assert(scr_info->info);
	for (i = 0; i < maxd->y+1; i++) {
		scr_info->info[i] = calloc(maxd->x+1, sizeof(struct pixinfo));
		assert(scr_info->info[i]);
	}
}

/* Relese memory occupied by scr_info */
void release_scr_info(struct screen *scr_info, struct maximumd *maxd)
{
	int i;

	for (i = 0; i < maxd->y+1; i++)
		free(scr_info->info[i]);
	free(scr_info->info);

}

/* Initialise stonearr and ship position for game */
void init_game(struct stonearr **sta, struct firearr **fira,
                     struct position *pos, struct maximumd *maxd)
{
	if (*sta != NULL)
		destroy_stonearr(*sta);
	*sta = create_stonearr();
	if (*fira != NULL)
		destroy_firearr(*fira);
	*fira = create_firearr();
	pos->y = maxd->y-2;
	pos->x = 10;
}

/* Release memory occupied by game */
void release_game(struct stonearr *sta, struct firearr *fira)
{
	if (sta != NULL)
		destroy_stonearr(sta);
	if (fira != NULL)
		destroy_firearr(fira);
}

/* Destroy stonearr sta */
void destroy_stonearr(struct stonearr *sta)
{
	int i;

	if (sta == NULL)
		return;

	for (i = 0; i < sta->nmemb; i++)
		free(sta->arr[i]);
	free(sta->arr);
	free(sta);
}

/* Destroy stonearr sta */
void destroy_firearr(struct firearr *fira)
{
	int i;

	if (fira == NULL)
		return;

	for (i = 0; i < fira->nmemb; i++)
		free(fira->arr[i]);
	free(fira->arr);
	free(fira);
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
void move_ship(struct vehicle *ship, struct position *pos, struct firearr *fira)
{
	int i;
	int c;

	/* timeout(10); */
	nodelay(stdscr, TRUE);
	i = 0;
	while (i++ < 20 && (c = getch()) != ERR) {
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
			fire(fira, ship);
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
	scr_info.info[newstone->pos.y][newstone->pos.x].sto = 1; 

	return newstone;
}

/* Run stones toward ship */
void run_stones(struct stonearr *sta, struct maximumd *maxd)
{
	int i;
	struct stone *s;

	for (i = 0; i < sta->nmemb; i++) {
		if ((s = sta->arr[i]) != NULL) {
			if (s->pos.y == maxd->y ||
			    scr_info.info[s->pos.y][s->pos.x].fir == 1 ||
			                           (s->pos.y < maxd->y &&
			    scr_info.info[s->pos.y+1][s->pos.x].fir == 1)) {
				scr_info.info[s->pos.y][s->pos.x].sto = 0;
				/* TODO: Fire zero all the time?  */
				scr_info.info[s->pos.y][s->pos.x].fir = 0;
				free(s); 
				sta->arr[i] = sta->arr[sta->nmemb-1]; 
				sta->arr[sta->nmemb-1] = NULL; 
				sta->nmemb--; 
			}
			else if (s->pos.y < maxd->y){
				scr_info.info[s->pos.y][s->pos.x].sto = 0; 
				(s->pos.y)++;
				scr_info.info[s->pos.y][s->pos.x].sto = 1; 
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
			if (color == 1) {
				attron(COLOR_PAIR(1));
				attron(A_BOLD);
			}
			printw("*");
			if (color == 1) {
				attroff(COLOR_PAIR(1));
				attroff(A_BOLD);
			}
		}
	}
	refresh();
}

/* Fire a shot */
void fire(struct firearr *fira, struct vehicle *v)
{
	struct position prev;
	struct fire *newfire;

	getyx(stdscr, prev.y, prev.x);
	move(v->firep.y, v->firep.x);
	newfire = mknew_fire(&v->firep);
	add_fire(fira, newfire);
	printw("^");
	scr_info.info[v->firep.y][v->firep.x].fir = 1; 
	move(prev.y, prev.x);
	refresh();
}

/* Create a new fire at a given pos */
struct fire *mknew_fire(struct position *pos)
{
	struct fire *newfire;

	newfire = malloc(sizeof(struct fire));
	assert(newfire);
	newfire->pos.y = pos->y;
	newfire->pos.x = pos->x;

	return newfire;
}

/* Add a newfire to firearr */
void add_fire(struct firearr *fira, struct fire *newfire)
{
	if (fira->nmemb == fira->cap)
		grow_firearr(fira);

	fira->arr[fira->nmemb++] = newfire;
}

/* Grow a firearr */
void grow_firearr(struct firearr *fira)
{
	struct fire **arr;

	arr = realloc(fira->arr, fira->cap * GROWTH * sizeof(struct fire *));
	assert(arr);
	fira->arr = arr;
	fira->cap *= GROWTH;
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
	if (color == 1) {
		attron(COLOR_PAIR(3));
		attron(A_BOLD);
	}
	for (i = 0; i < SHIP_HEIGHT; i++) {
		move(p->y-i, p->x);
		printw("%s", ship->body[SHIP_HEIGHT-i-1]);
	}
	if (color == 1) {
		attroff(COLOR_PAIR(3));
		attroff(A_BOLD);
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

/*
 * BUGS
 * ====
 *
 * 1. Key repeat delay causes issue when user wants to move ship by more than
 *    one position.
 *
 * 2. When `f' is pressed to fire, the movement key is ignored. So the user has
 *    to again press the movement key after pressing fire key.
 *
 * 3. When game is over, screen is in cleared state. Instead it should should
 *    be in the the current state (with the stones and fires).
 *
 * 4. burn_ship function to too miniaml. It should be more natural.
 *
 * 5. Game does not end when ship touches boundary of window. 
 */

