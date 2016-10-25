/*
 * tic_tac_toe.c: My own tic tac toe C Source code
 *
 * St: 2016-10-10 Mon 02:07 AM
 * Up: 2016-10-10 Mon 03:54 AM
 *
 * Author: SPS
 */

#include<stdio.h>

#define SKIP
#define LEFT_MARGIN 10

/* Function prototype declarations */
void print_state(int (*blocks)[9]);
void print_left_margin(void);
int validate_choice(int choice, int (*blocks)[9]);
int win(char user, int (*blocks)[9]);
void update_blocks(char user, int choice, int (*blocks)[9]);
int print_err_msg(int choice);

/* main: start */
int main(void)
{
	int i;
	int choice;
	char user;
	int blocks[2][9] = {0};

	print_state(blocks);

	for (i = 0; i < 9; i++) {
		if (i % 2 == 0)
			user = 'A';
		else
			user = 'B';
		do {
			printf("User %c, select a box:  ", user);
			scanf("%d", &choice);
		} while (validate_choice(choice, blocks) != 1 &&
		         print_err_msg(choice));

		update_blocks(user, choice, blocks);

		print_state(blocks);

		if (win(user, blocks) == 1) {
			printf("Game over, User %c won!!!\n", user);
			break;
		}
	}

	if (i == 9)
		printf("Game Draw !!!\n");

	return 0;
}

/*
 * Print error message.
 *
 * @choice: Choice made by user
 */
int print_err_msg(int choice)
{
	printf("\nError: Invalid choice: %d, try again.\n\n", choice);

	return 1;
}

/*
 * Update already chosen blocks info.
 *
 * @user:
 * @choice:
 * @blocks:
 */
void update_blocks(char user, int choice, int (*blocks)[9])
{
	if (user == 'A')
		blocks[0][choice-1] = 1;
	else
		blocks[1][choice-1] = 1;
}

/*
 * Validate users choice.
 *
 * @choice:      Choice
 * @blocks: Array containing user box state
 */
int validate_choice(int choice, int (*blocks)[9])
{
	int retval;

	if (choice < 1 || choice > 9)
		retval = 0;
	else if (blocks[0][choice-1] == 1 || blocks[1][choice-1] == 1)
		retval = 0;
	else
		retval = 1;
	
	return retval;
}

/*
 * Check if a user has won the game.
 *
 * @user:
 * @blocks:
 */
int win(char user, int (*blocks)[9])
{
	int i;
	int u;
	int retval;

	if (user == 'A')
		u = 0;
	else
		u = 1;

	retval = 0;

	/* Check diagonals */
	if (blocks[u][0] && blocks[u][4] && blocks[u][8]) {
		retval = 1;
	} else if (blocks[u][2] && blocks[u][4] && blocks[u][6]) {
		retval = 1;
	} else {
	/* Check for rows and columns  */
		for (i = 0; i < 3; i++) {
			/* Check row wise */
			if (blocks[u][3*i + 0] && 
			    blocks[u][3*i + 1] && blocks[u][3*i + 2]) {
				retval = 1;   
				break;
			}
			/* Check col wise */
			if (blocks[u][i] &&
			    blocks[u][i+3] && blocks[u][i+6]) {
				retval = 1;
				break;
			}
		}
	}

	return retval;
}

/*
 * Print the current state of game 
 *
 * @blocks: Array containing info about users blocks
 */
void print_state(int (*blocks)[9])
{
	int i;
	int j;

	char linesp[6] = "-----";
	char lines[3][6] = {
				{'1', '|', '2', '|', '3', '\0'},
				{'4', '|', '5', '|', '6', '\0'},
				{'7', '|', '8', '|', '9', '\0'}
			   };

	/* Set state */
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			if (blocks[0][3*i + j] == 1)
				lines[i][2*j] = 'X';
			if (blocks[1][3*i + j] == 1)
				lines[i][2*j] = 'O';
			
		}
	}

	/* Print state */
	putchar('\n');
	for (i = 0; i < 3; i++) {
		print_left_margin();
		printf("%s\n", lines[i]);
		if (i != 2) {
			print_left_margin();
			printf("%s\n", linesp);
		}
	}
	putchar('\n');
}

/* Print left margin of blank spaces */
void print_left_margin(void)
{
	int i;

	for (i = 0; i < LEFT_MARGIN; i++)
		putchar(' ');
}

