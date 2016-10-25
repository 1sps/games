/*
 * hangman.c: Hangman game implementation in C language.
 *
 * St: 2016-10-10 Mon 04:40 AM
 * Up: 2016-10-11 Tue 12:19 PM
 *
 * Author: SPS
 */

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<assert.h>
#include<ctype.h>
#include<ncurses.h>
#include<time.h>
#include"mnlib.h"

#define SKIP
#define MAX_MISTAKES 6
#define FILENAME "/usr/share/dict/words"

struct alpha {
	char letter;
	int count;
	int *indices;
};

enum line_nums {
	DISP_LNO  = 1,
	PINFO_LNO = 7,
	INC_LNO   = 10,
	NEXTG_LNO = 12,
	MIS_LNO   = 14,
	USED_LNO  = 16,
	FIN_LNO   = 18,
	APA_LNO   = 21,
	ERR_LNO   = 26
};


/* Function prototype declaratoins */
char *get_word(char *filename);
int get_line_count(char *filename);
void print_info(char *word, int pinfo_lno);
void print_fail(char *word, char *answer, int lno);
void print_inc_answer(char *answer, int len, int lno);
void print_success(char *answer, int lno);
void print_emsg(void);
int is_mistake(int c, char *answer, struct alpha *info );
int word_complete(char *word, char *answer);
struct alpha *setup_info(char *word);
int get_guess(int maxy);
void word_tolower(char *word);
int hangman(void);
void print_mistake_count(int mistake, int lno);
void update_used_letters(char *used, int c);
void print_used_letters(char *used, int used_lno);
int ask_play_again(int lno);
void print_init_screen(char *word, char *answer, char *usedl, int mistake);
void update_answer(int c, char *answer, struct alpha *info);
void update_disp(int mistake);
void print_disp(void);

/* main: start */
int main(void)
{
	int again;

	m_init_scr();

	do {
		again = hangman();
	} while (again == 'y');

	m_end_win();

	return 0;
}

/* Hangman */
int hangman(void)
{
	int c;
	int len;
	int again;
	int mistake;
	char *word;
	char *answer;
	struct alpha *info;
	char usedl[26] = {0};

	if ((word = get_word(FILENAME)) == NULL)
		exit(1);
	if ((len = strlen(word)) < 1)
		exit(1);
	answer = calloc(len+1, sizeof(char));
	assert(answer);

	mistake = 0;

	info = setup_info(word);

	clear();
	print_init_screen(word, answer, usedl, mistake);

	while (mistake < MAX_MISTAKES) {
		if ((c = get_guess(NEXTG_LNO)) == -1) {
			print_emsg();
			exit(1);
		}
		if (is_mistake(c, answer, info) == 1) {
			mistake++;
			update_disp(mistake);
		} else {
			update_answer(c, answer, info);
			if (word_complete(word, answer) == 1) {
				print_inc_answer(answer, len, INC_LNO);
				print_success(answer, FIN_LNO);
				break;
			}
		}

		update_used_letters(usedl, c);
		print_used_letters(usedl, USED_LNO);
		print_mistake_count(mistake, MIS_LNO);
		print_disp();
	}

	if (mistake == MAX_MISTAKES)
		print_fail(word, answer, FIN_LNO);

	again = ask_play_again(APA_LNO);

	free(word);
	free(answer);

	return again;
}

void print_init_screen(char *word, char *answer, char *usedl, int mistake)
{
	size_t len;
	int prevy;
	int prevx;

	len = strlen(word);

	getyx(stdscr, prevy, prevx);

	print_disp();
	print_info(word, PINFO_LNO);
	print_inc_answer(answer, len, INC_LNO);
	print_used_letters(usedl, USED_LNO);
	print_mistake_count(mistake, MIS_LNO);

	move(prevy, prevx);
}

int ask_play_again(int lno)
{
	int c;

	move(lno, 0);
	clrtoeol();
	refresh();
	printw("Restart ? [y\\n] : ");
	refresh();

	nocbreak();
	echo();
	c = getch();
	getch();
	noecho();
	cbreak();

	move(lno, 0);
	clrtoeol();
	refresh();

	return c;
}

void update_used_letters(char *usedl, int c)
{
	int idx;

	idx = c - 'a';

	usedl[idx] = 1;
}

void print_used_letters(char *usedl, int used_lno)
{
	int i;

	move(used_lno, 0);
	clrtoeol();
	refresh();
	printw("Used letters: ");
	for (i = 0; i < 26; i++) {
		if (usedl[i] != 0)
			printw("%c ", i + 'a');
	}
	refresh();
}

void print_mistake_count(int mistake, int lno)
{
	move(lno, 0);
	printw("Mistake count: %d", mistake);
	refresh();
}

void word_tolower(char *word)
{
	int i;
	int len;

	len = strlen(word);
	for (i = 0; i < len; i++)
		word[i] = tolower(word[i]);
}

void print_fail(char *word, char *answer, int lno)
{
	int i;
	int len;

	len = strlen(word);

	for (i = 0; i < len; i++) {
		if (answer[i] == 0)
			answer[i] = '_';
	}
	
	move(lno, 0);
	printw("Fail !!! Game over!");
	move(lno+1, 0);
	printw("Word is: %s", word);
}

/*
 * Print incomplete answer.
 *
 * @len: Length of the correct word
 */
void print_inc_answer(char *answer, int len, int lno)
{
	int i;

	move(lno, 0);
	clrtoeol();
	printw("Incomplete answer: ");
	for (i = 0; i < len; i++) {
		if (answer[i] == 0)
			printw("_");
		else
			printw("%c", answer[i]);
	}
	refresh();
}

void print_success(char *answer, int lno)
{
	move(lno, 0);
	printw("Succes! You completed the word");
	move(lno+1, 0);
	printw("Word is: %s", answer);
	refresh();
}

void print_emsg(void)
{
	fprintf(stderr, "hangman: Error: Program crash ...\n");
}

/* Check if guess is a mistake */
int is_mistake(int c, char *answer, struct alpha *info )
{
	int idx;
	int count;
	int mistake;

	idx = c - 'a';

	if ((count = info[idx].count) == 0)
		mistake = 1;
	else
		mistake = 0;

	return mistake;
}

void update_answer(int c, char *answer, struct alpha *info)
{
	int i;
	int idx;
	int count;

	idx = c - 'a';

	count = info[idx].count;

	for (i = 0; i < count; i++) {
		if (answer[info[idx].indices[i]] == 0)
			answer[info[idx].indices[i]] = c;
	}
}

int word_complete(char *word, char *answer)
{
	if (strncmp(word, answer, strlen(word)+1) == 0)
		return 1;
	else
		return 0;
}

struct alpha *setup_info(char *word)
{
	int i;
	int idx;
	size_t len;
	struct alpha *info;

	info = malloc(sizeof(struct alpha) * 26);
	assert(info);

	for (i = 0; i < 26; i++) {
		info[i].letter = 'a' + i;
		info[i].count = 0;
		info[i].indices = NULL;
	}

	len = strlen(word);
	for (i = 0; i < len; i++) {
		idx = word[i] - 'a';
		info[idx].count++; 
	        info[idx].indices = realloc(info[idx].indices,
		                            sizeof(int) * info[idx].count);
		assert(info[idx].indices);
		info[idx].indices[info[idx].count-1] = i;
	}

	return info;
}

int get_guess(int nextg_lno)
{
	int c;
	int prevy;
	int prevx;

	getyx(stdscr, prevy, prevx);
	move(nextg_lno, 0);
	clrtoeol();
	refresh();
	printw("Guess next letter: ");
	refresh();

	nocbreak();
	echo();
	c = getch();
	getch();
	noecho();
	cbreak();

	move(nextg_lno, 0);
	clrtoeol();
	refresh();

	move(prevy, prevx);
	refresh();

	return c;
}

void print_info(char *word, int pinfo_lno)
{
	move(pinfo_lno, 0);
	printw("Word length: %zu", strlen(word));
	move(pinfo_lno+1, 0);
	printw("Max number of mistakes: %d", MAX_MISTAKES);
	refresh();
}

/*
 * Get a random word from file
 *
 * @fp: Pointer to file
 */
char *get_word(char *filename)
{
	char line[100];
	char *token;
	char *word;
	int line_count;
	int line_nmbr;
	FILE *fp;

	/* Count total lines */
	line_count = get_line_count(filename);
	if (line_count == 0)
		return NULL;

	/*
	 * Select a random line number
	 */
	srand(time(NULL));
	line_nmbr = rand() % line_count;
	/* line number range is: 1 to line_count */
	line_nmbr++;

	/* Open file and get that line */
	fp = fopen(filename, "r");
	if (fp == NULL) {
		move(ERR_LNO, 0);
		printw("hangman: Error: Could not open dictionary");
		refresh();
		return NULL;
	}
	while (line_nmbr-- > 0) {
		if (fgets(line, 100, fp) == NULL) {
			move(ERR_LNO, 0);
			printw("hangman: Error: Could not read word");
			refresh();
			fclose(fp);
			return NULL;
		}
	}
	fclose(fp);

	/* Get first word from that line */
	if (line[strlen(line)-1] == '\n')
		line[strlen(line)-1] = '\0';
	token = strtok(line, " ");
	word = strdup(token);
	word_tolower(word);
	if (word == NULL) {
		move(ERR_LNO, 0);
		printw("hangman: Error: Could not get word");
		refresh();
	}
			
	return word;
}

/*
 * Get line count of a file.
 *
 * @filename: Name of the file
 */
int get_line_count(char *filename)
{
	char line[100];
	int line_count;
	FILE *fp;

	line_count = 0;

	fp = fopen(filename, "r");
	if (fp != NULL) {
		while (fgets(line, 100, fp) != NULL)
			line_count++;
		fclose(fp);
	}

	return line_count;
}

char disp1[] = "_______________________";
char disp2[] = "                |      ";
char disp3[] = "                       ";
char disp4[] = "                       ";
char disp5[] = "                       ";

#ifndef SKIP
/* This is how it will look when game is over */
char disp1[] = "_______________________";
char disp2[] = "                |      ";
char disp3[] = "                O      ";
char disp4[] = "               /|\\    ";
char disp5[] = "               / \\    ";
#endif /* SKIP */

void update_disp(int mistake)
{
	switch(mistake) {
	case(1):
		disp3[16] = 'O';
		break;
	case(2):
		disp4[15] = '/';
		break;
	case(3):
		disp4[16] = '|';
		break;
	case(4):
		disp4[17] = '\\';
		break;
	case(5):
		disp5[15] = '/';
		break;
	case(6):
		disp5[17] = '\\';
		break;
	default:
		break;
	}
}

void print_disp(void)
{
	move(DISP_LNO, 0);
	printw("%s", disp1);
	move(DISP_LNO+1, 0);
	printw("%s", disp2);
	move(DISP_LNO+2, 0);
	printw("%s", disp3);
	move(DISP_LNO+3, 0);
	printw("%s", disp4);
	move(DISP_LNO+4, 0);
	printw("%s", disp5);
	move(DISP_LNO+5, 0);
	refresh();
}
	      
/*
 * Issues
 *
 * 1. Segfaults randomly, especially when playing again.
 *
 * Status: Under debug.
 */

