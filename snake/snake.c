/*
 * snake.c: Implementation of snake game in C.
 *
 * St: 2016-10-21 Fri 09:17 PM
 * Up: 2016-10-25 Tue 01:48 AM
 *
 * Author: SPS
 *
 * License: BSD
 */

#include<stdio.h>
#include<time.h>
#include<stdlib.h>
#include<ncurses.h>
#include<unistd.h>
#include<assert.h>
#include"mnlib.h"
#include"mylib.h"

#define SKIP

#define SNK_WIN_HEIGHT  20
#define SNK_WIN_WIDTH   40
#define SNK_WIN_STARTY  5
#define SNK_WIN_STARTX  2

#define SCORE_WIN_HEIGHT  4
#define SCORE_WIN_WIDTH   40
#define SCORE_WIN_STARTY  1
#define SCORE_WIN_STARTX  2

#define UIP_WIN_HEIGHT  3
#define UIP_WIN_WIDTH   40
#define UIP_WIN_STARTY  26
#define UIP_WIN_STARTX  2

#define SNAKE_INIT_LEN 6
#define SNAKE_INIT_DIR 'l'
#define SNAKE_INIT_SPEED 1

#define BODY_POS 1
#define FREE_POS 2
#define FOOD_POS 3

#define INIT_INTERVAL 100000

#define SCORE_INC 10

struct pos {
	int y;
	int x;
};

struct snake {
	struct pos start; 
	int len;
	int dir;
	struct deque *body;
	int speed;
};

/* Function prototype declarations */
WINDOW *create_new_win(int height, int width, int sty, int stx);
void boxify(WINDOW *win);
struct snake *create_snake(int len, int dir, int speed);
void print_snake(struct snake *sk, WINDOW *gwin);
void erase_snake(struct snake *sk, WINDOW *gwin);
void *cpy_pos(void *pos);
void dval_pos(void *pos);
int get_dir(WINDOW *gwin);
void update_snake_dir(struct snake *sk, int dir);
void move_snake(struct snake *sk, int *len_inc);
int valid_dir(struct snake *sk, int dir);
void init_board(int board[][SNK_WIN_WIDTH]);
void update_board(struct snake *sk, int board[][SNK_WIN_WIDTH]);
int snake_crash(struct snake *sk, int board[][SNK_WIN_WIDTH]);
void init_rand(void);
struct pos get_food_pos(int board[][SNK_WIN_WIDTH]);
void print_food(struct pos *food_pos, WINDOW *gwin);
int eat_food(struct snake *sk, struct pos *food_pos);
int pos_equal(struct pos *p, struct pos *q);
void destroy_snake(struct snake *sk);
void play_game(WINDOW *snk_win, WINDOW *score_win, int board[][SNK_WIN_WIDTH]);
void start_game(WINDOW *snk_win, WINDOW *score_win,
                struct snake **sk, int board[][SNK_WIN_WIDTH]);
void run_game(WINDOW *snk_win, WINDOW *score_win,
              struct snake *sk, int board[][SNK_WIN_WIDTH]);
void init_env(WINDOW **snk_win, WINDOW **score_win, WINDOW **uip_win);
void destroy_env(WINDOW *snk_win, WINDOW *score_win, WINDOW *uip_win);
int play_again(WINDOW *uip_win);
void clear_win(WINDOW *win, int box);
void print_crashed_snake(struct snake *sk, WINDOW *gwin);
int inc_score(int score);
void print_score(WINDOW *score_win, int score);
void end_game(struct snake *sk);
void run_game_new(WINDOW *snk_win, WINDOW *score_win,
              struct snake *sk, int board[][SNK_WIN_WIDTH]);
int update_level(int score, int *level);
void print_level(WINDOW *score_win, int level);
int update_interval(int interval);

/* main: start */
int main(void)
{
	WINDOW *snk_win;
	WINDOW *score_win;
	WINDOW *uip_win;
	int board[SNK_WIN_HEIGHT][SNK_WIN_WIDTH];

	m_init_scr();
	init_rand();
	init_env(&snk_win, &score_win, &uip_win);

	do 
		play_game(snk_win, score_win, board);
	while (play_again(uip_win) == 1);

	destroy_env(snk_win, score_win, uip_win);
	m_end_win();

	return 0;
}

/* Ask if play again */
int play_again(WINDOW *uip_win)
{
	int c;
	int retval;

	flushinp();
	werase(uip_win);
	mvwprintw(uip_win, 1, 1, "Play again? : [y/n] : ");
	wrefresh(uip_win);

	echo();
	nocbreak();
	c = wgetch(uip_win);
	cbreak();
	noecho();

	flushinp();
	wmove(uip_win, 1, 1);
	wclrtoeol(uip_win);
	wrefresh(uip_win);
	move(0, 0);
	refresh();

	if (c == 'y')
		retval = 1;
	else
		retval = 0;

	return retval;
}

/* Destroy game environment */
void destroy_env(WINDOW *snk_win, WINDOW *score_win, WINDOW *uip_win)
{
	delwin(snk_win);
	delwin(score_win);
	delwin(uip_win);
}

/* Initialize environment to play game */
void init_env(WINDOW **snk_win, WINDOW **score_win, WINDOW **uip_win)
{
	*snk_win = create_new_win(SNK_WIN_HEIGHT, SNK_WIN_WIDTH,
	                          SNK_WIN_STARTY, SNK_WIN_STARTX);

	*score_win = create_new_win(SCORE_WIN_HEIGHT, SCORE_WIN_WIDTH,
	                            SCORE_WIN_STARTY, SCORE_WIN_STARTX);

	*uip_win = create_new_win(UIP_WIN_HEIGHT, UIP_WIN_WIDTH,
                                  UIP_WIN_STARTY, UIP_WIN_STARTX);
}

/* Play snake game */
void play_game(WINDOW *snk_win, WINDOW *score_win, int board[][SNK_WIN_WIDTH])
{
	struct snake *sk;

	start_game(snk_win, score_win, &sk, board);
	run_game_new(snk_win, score_win, sk, board);
	end_game(sk);
}

/* Start a new instance of game */
void start_game(WINDOW *snk_win, WINDOW *score_win,
                struct snake **sk, int board[][SNK_WIN_WIDTH])
{
	clear_win(snk_win, 1);
	clear_win(score_win, 1);
	*sk = create_snake(SNAKE_INIT_LEN, SNAKE_INIT_DIR, SNAKE_INIT_SPEED);
	update_board(*sk, board);
}

/*
 * Clear a window in screen. 
 *
 * Also, redraw the box around window if needed.
 */
void clear_win(WINDOW *win, int box)
{
	wmove(win, 0, 0);
	wclrtobot(win);
	if (box == 1) {
		boxify(win);
		refresh();
		wrefresh(win);
	}
}

/* Run snake game */
void run_game(WINDOW *snk_win, WINDOW *score_win,
              struct snake *sk, int board[][SNK_WIN_WIDTH])
{
	int dir;
	int score;
	int len_inc;
	struct pos food_pos;

	score = 0;
	len_inc = 0;
	food_pos = get_food_pos(board);
	board[food_pos.y][food_pos.x] = FOOD_POS;
	print_food(&food_pos, snk_win);
	print_snake(sk, snk_win);
	print_score(score_win, score);

	while (1) {
		usleep(100000);

		dir = get_dir(snk_win);
		if (valid_dir(sk, dir))
			update_snake_dir(sk, dir);

		erase_snake(sk, snk_win);
		move_snake(sk, &len_inc);
		if (snake_crash(sk, board)) {
			print_crashed_snake(sk, snk_win);
			break; 
		} 
		update_board(sk, board); 
		if (eat_food(sk, &food_pos)) {
			food_pos = get_food_pos(board);
			board[food_pos.y][food_pos.x] = FOOD_POS;
			score = inc_score(score);
			print_score(score_win, score);
			len_inc = 2;
		}
		print_food(&food_pos, snk_win);
		print_snake(sk, snk_win);

		move(0, 0);
		refresh();
	}
}

/* Run snake game */
void run_game_new(WINDOW *snk_win, WINDOW *score_win,
              struct snake *sk, int board[][SNK_WIN_WIDTH])
{
	int dir;
	int score;
	int len_inc;
	int level;
	int interval;
	struct pos food_pos;

	score = 0;
	len_inc = 0;
	level = 0;
	interval = INIT_INTERVAL;
	food_pos = get_food_pos(board);
	board[food_pos.y][food_pos.x] = FOOD_POS;
	print_food(&food_pos, snk_win);
	print_snake(sk, snk_win);
	print_score(score_win, score);
	print_level(score_win, level);

	while (1) {
		usleep(interval);

		dir = get_dir(snk_win);
		if (valid_dir(sk, dir))
			update_snake_dir(sk, dir);

		erase_snake(sk, snk_win);
		move_snake(sk, &len_inc);
		if (snake_crash(sk, board)) {
			print_crashed_snake(sk, snk_win);
			break; 
		} 
		update_board(sk, board); 
		if (eat_food(sk, &food_pos)) {
			food_pos = get_food_pos(board);
			board[food_pos.y][food_pos.x] = FOOD_POS;
			score = inc_score(score);
			if (update_level(score, &level) == 1)
				interval = update_interval(interval);
			print_score(score_win, score);
			print_level(score_win, level);
			len_inc = 2;
		}
		print_food(&food_pos, snk_win);
		print_snake(sk, snk_win);

		move(0, 0);
		refresh();
	}
}

#define INTERVAL_DEC 10000
/* Update interval */
int update_interval(int interval)
{
	return interval -= INTERVAL_DEC;
}

#define LEVEL_POINTS 100
/*
 * Update game level. Level changes after every LEVEL_POINTS.
 * Returns 1 if level changes, else returns 0.
 */
int update_level(int score, int *level)
{
	int retval;

	if (score % LEVEL_POINTS == 0) {
		(*level)++;
		retval = 1;
	} else {
		retval = 0;
	}

	return retval;
}


/* Increment score */
int inc_score(int score)
{
	return score + SCORE_INC;
}

/* Print score */
void print_score(WINDOW *score_win, int score)
{
	mvwprintw(score_win, 1, 2, "Score: %d", score);
	wrefresh(score_win);
}

/* Print level */
void print_level(WINDOW *score_win, int level)
{
	int y;

	y = SCORE_WIN_WIDTH - 10;
	mvwprintw(score_win, 1, y, "Level: %d", level);
	wrefresh(score_win);
}

/* End game */
void end_game(struct snake *sk)
{
	destroy_snake(sk);
}

/* Destroy snake */
void destroy_snake(struct snake *sk)
{
	deque_destroy(sk->body);
	free(sk);
}

/* Check if snake eats food */
int eat_food(struct snake *sk, struct pos *food_pos)
{
	void *itr;
	int retval;
	struct pos *head_pos;

	itr = deque_begin(sk->body);
	if (!deque_done(itr)) {
		head_pos = deque_next(sk->body, &itr);
	} else {
		abort();
	}
	
	if (pos_equal(head_pos, food_pos)) {
		retval = 1;
	} else {
		retval = 0;
	}

	free(head_pos);

	return retval;
}

/* Check if two positions are equal */
int pos_equal(struct pos *p, struct pos *q)
{
	int retval;
	
	if (p->y == q->y && p->x == q->x)
		retval = 1;
	else
		retval = 0;

	return retval;
}

/* Initialize random number generator */
void init_rand(void)
{
	srand(time(NULL));
}

/* Get a new position for food */
struct pos get_food_pos(int board[][SNK_WIN_WIDTH])
{
	struct pos food_pos;

	do {
		food_pos.y = (rand() % (SNK_WIN_HEIGHT-2)) + 1;
		food_pos.x = (rand() % (SNK_WIN_WIDTH-2)) + 1;
	}
	while (board[food_pos.y][food_pos.x] != FREE_POS);

	return food_pos;
}

/* Print food */
void print_food(struct pos *food_pos, WINDOW *gwin)
{
	mvwprintw(gwin, food_pos->y, food_pos->x, "#");
	refresh();
	wrefresh(gwin);
}

/* Check if snake has crashed */
int snake_crash(struct snake *sk, int board[][SNK_WIN_WIDTH])
{
	int retval;
	void *itr;
	struct pos *sk_head;

	retval = 0;

	itr = deque_begin(sk->body);
	if (!deque_done(itr)) {
		sk_head = deque_next(sk->body, &itr);
		if (board[sk_head->y][sk_head->x] == BODY_POS)
			retval = 1;
	} else {
		abort();
	}

	free(sk_head);

	return retval;
}

/* Initialize board */
void init_board(int board[][SNK_WIN_WIDTH])
{
	int i;
	int j;

	for (i = 0; i < SNK_WIN_HEIGHT; i++) {
		for (j = 0; j < SNK_WIN_WIDTH; j++)
			board[i][j] = FREE_POS;
	}
}

/* Update board */
void update_board(struct snake *sk, int board[][SNK_WIN_WIDTH])
{
	void *itr;
	struct pos *cur;

	init_board(board);

	itr = deque_begin(sk->body);
	while (!deque_done(itr)) {
		cur = deque_next(sk->body, &itr);
		board[cur->y][cur->x] = BODY_POS;
		free(cur);
	}
}

/* Validate direction request */
int valid_dir(struct snake *sk, int dir) 
{
	int valid;

	valid = 0;

	switch (dir) {
	case 'h':
		if (sk->dir != 'l')
			valid = 1;
		break;
	case 'j':
		if (sk->dir != 'k')
			valid = 1;
		break;
	case 'k':
		if (sk->dir != 'j')
			valid = 1;
		break;
	case 'l':
		if (sk->dir != 'h')
			valid = 1;
		break;
	default:
		break;
	}

	return valid;
}

/* Get direction request from user */
int get_dir(WINDOW *gwin)
{
	int c;

	nodelay(gwin, TRUE);
	c = wgetch(gwin);
	flushinp();
	timeout(-1);

	return c;
}

/* Update snake dir */
void update_snake_dir(struct snake *sk, int dir)
{
	sk->dir = dir;
}

/* Move snake */
void move_snake(struct snake *sk, int *len_inc)
{
	int dir;
	void *itr;
	struct pos *p;
	struct pos new_sk_head;

	/* Remove last position of snake body if needed */
	if (*len_inc == 0) {
		p = deque_pop(sk->body, DEQUE_TAIL);
		free(p);
	} else {
		(*len_inc)--;
	}

	/*
	 * Update (add) new head position
	 */

	itr = deque_begin(sk->body);
	if (!deque_done(itr))
		p = deque_next(sk->body, &itr);
	else
		abort();
	dir = sk->dir;
	switch (dir) {
	case 'h':
		new_sk_head.y = p->y;
		if (p->x-1 == 0)
			new_sk_head.x = SNK_WIN_WIDTH-2;
		else
			new_sk_head.x = p->x-1;
		break;
	case 'j':
		if (p->y+1 == SNK_WIN_HEIGHT-1)
			new_sk_head.y = 1;
		else
			new_sk_head.y = p->y+1;
		new_sk_head.x = p->x;
		break;
	case 'k':
		if (p->y-1 == 0)
			new_sk_head.y = SNK_WIN_HEIGHT-2;
		else
			new_sk_head.y = p->y-1;
		new_sk_head.x = p->x;
		break;
	case 'l':
		new_sk_head.y = p->y;
		if (p->x+1 == SNK_WIN_WIDTH-1)
			new_sk_head.x = 1;
		else
			new_sk_head.x = p->x+1;
		break;
	default:
		break;
	}

	free(p);

	deque_insert(sk->body, DEQUE_HEAD, &new_sk_head);
}

/* Create a new window */
WINDOW *create_new_win(int height, int width, int sty, int stx)
{
	WINDOW *nwin;

	nwin = newwin(height, width, sty, stx);
	assert(nwin);

	return nwin;
}

/* Put a box around window */
void boxify(WINDOW *win)
{
	box(win, 0, 0);
}

/* Create a new snake */
struct snake *create_snake(int len, int dir, int speed)
{
	int i;
	struct pos p;
	struct snake *sk;

	sk = malloc(sizeof(struct snake));
	assert(sk);

	sk->len = len;
	sk->dir = dir;
	sk->speed = speed;

	sk->body = deque_create(cpy_pos, NULL, dval_pos, NULL);
	for (i = 0; i < len; i++) {
		p.y = 2;
		p.x = i+1;
		deque_insert(sk->body, DEQUE_HEAD, &p);
	}

	return sk;
}

/* Erase snake in the game window */
void erase_snake(struct snake *sk, WINDOW *gwin)
{
	void *itr;
	struct pos *cur;

	itr = deque_begin(sk->body);
	while (!deque_done(itr)) {
		cur = deque_next(sk->body, &itr);
		mvwprintw(gwin, cur->y, cur->x, " ");
		free(cur);
	}

	refresh();
	wrefresh(gwin);
}

/* Print snake in the game window */
void print_snake(struct snake *sk, WINDOW *gwin)
{
	void *itr;
	struct pos *cur;

	itr = deque_begin(sk->body);
	while (!deque_done(itr)) {
		cur = deque_next(sk->body, &itr);
		mvwprintw(gwin, cur->y, cur->x, "*");
		free(cur);
	}

	refresh();
	wrefresh(gwin);
}

/* Print snake in the game window */
void print_crashed_snake(struct snake *sk, WINDOW *gwin)
{
	int i;
	void *itr;
	struct pos *cur;

	print_snake(sk, gwin);

	itr = deque_begin(sk->body);
	if (!deque_done(itr))
		cur = deque_next(sk->body, &itr);
	else
		abort();

	curs_set(0);
	wmove(gwin, cur->y, cur->x);
	for (i = 0; i < 4; i++) {
		wchgat(gwin, 1, A_BOLD, 3, NULL);
		wrefresh(gwin);
		usleep(300000);
		wchgat(gwin, 1, A_NORMAL, 4, NULL);
		wrefresh(gwin);
		usleep(300000);
	}
	wchgat(gwin, 1, A_BOLD, 3, NULL);
	wrefresh(gwin);
	curs_set(1);
	move(0, 0);
	refresh();

	free(cur);
}


/* Destroy a pos node in pos deque */
void dval_pos(void *pos)
{
	free(pos);
}

/* Make a copy of pos */
void *cpy_pos(void *pos)
{
	struct pos *newpos;
	struct pos *srcpos;

	srcpos = (struct pos *) pos;

	newpos = malloc(sizeof(struct pos));
	assert(newpos);

	newpos->y = srcpos->y;
	newpos->x = srcpos->x;

	return newpos;
}

