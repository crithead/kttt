/*
 * A procfs tic-tac-toe game and general playground for developing kernel
 * loadable modules in Linux 3.10 and later.
 *
 * - Uses the procfs API in 3.10+
 * - Implements read and write.
 */

#ifndef __KERNEL__
  #define __KERNEL__
  #define MODULE
#endif

#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/ctype.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("donp");
MODULE_DESCRIPTION("In-kernel tic-tac-toe");

/* ************************************************************************ */
/* Game logic */

#define TTT_NONE '-'
#define TTT_OVER '+'
#define TTT_O 'O'
#define TTT_X 'X'
#define TTT_WINNER 0
static char ttt_game[10]; /* pos 0 is winner */
static char ttt_turn = TTT_NONE;
static char ttt_count = 0;

static void ttt_reset(void)
{
	memset(ttt_game, TTT_NONE, sizeof(char) * 10);
	ttt_turn = TTT_O; /* O always goes first */
	ttt_count = 0;
}

static void ttt_check_winner(void)
{
	if ((ttt_game[1] == ttt_game[5]) &&
	    (ttt_game[5] == ttt_game[9]) &&
	    (ttt_game[9] == TTT_NONE)) {
		ttt_game[TTT_WINNER] = TTT_NONE;
	} else if ((ttt_game[1] == ttt_game[2]) &&
		   (ttt_game[2] == ttt_game[3]) &&
		   (ttt_game[3] != TTT_NONE)) {
		ttt_game[TTT_WINNER] = ttt_game[1];
	} else 	if ((ttt_game[4] == ttt_game[5]) &&
		    (ttt_game[5] == ttt_game[6]) &&
		    (ttt_game[6] != TTT_NONE)) {
		ttt_game[TTT_WINNER] = ttt_game[4];
	} else 	if ((ttt_game[7] == ttt_game[8]) &&
		    (ttt_game[8] == ttt_game[9]) &&
		    (ttt_game[9] != TTT_NONE)) {
		ttt_game[TTT_WINNER] = ttt_game[7];
	} else 	if ((ttt_game[1] == ttt_game[4]) &&
		    (ttt_game[4] == ttt_game[7]) &&
		    (ttt_game[7] != TTT_NONE)) {
		ttt_game[TTT_WINNER] = ttt_game[1];
	} else 	if ((ttt_game[2] == ttt_game[5]) &&
		    (ttt_game[5] == ttt_game[8]) &&
		    (ttt_game[8] != TTT_NONE)) {
		ttt_game[TTT_WINNER] = ttt_game[2];
	} else 	if ((ttt_game[3] == ttt_game[6]) &&
		    (ttt_game[6] == ttt_game[9]) &&
		    (ttt_game[9] != TTT_NONE)) {
		ttt_game[TTT_WINNER] = ttt_game[3];
	} else 	if ((ttt_game[1] == ttt_game[5]) &&
		    (ttt_game[5] == ttt_game[9]) &&
		    (ttt_game[9] != TTT_NONE)) {
		ttt_game[TTT_WINNER] = ttt_game[1];
	} else 	if ((ttt_game[3] == ttt_game[5]) &&
		    (ttt_game[5] == ttt_game[7]) &&
		    (ttt_game[7] != TTT_NONE)) {
		ttt_game[TTT_WINNER] = ttt_game[3];
	} else {
		ttt_game[TTT_WINNER] = TTT_NONE;
	}

	if (ttt_game[TTT_WINNER] != TTT_NONE)
		ttt_turn = TTT_NONE;
	if (ttt_count >= 9)
		ttt_turn = TTT_OVER;
}

static void ttt_take_turn(char player, int pos)
{
	if (ttt_turn != player)
		return;
	if (pos < 1 || pos > 9)
		return;
	if (ttt_game[pos] == TTT_NONE) {
		ttt_game[pos] = player;
		ttt_turn = (ttt_turn == TTT_O) ? TTT_X : TTT_O;
		ttt_count++;
	}
	ttt_check_winner();
}

/* ************************************************************************ */
/* Proc file */

#define TTT_PROC_NAME "ttt"

static struct proc_dir_entry *ttt_proc;
static const mode_t TTT_MODE = 0666;

/* Proc file contents */
#define TTT_OUTPUT_LENGTH 36
static char ttt_output[TTT_OUTPUT_LENGTH];
static size_t ttt_output_size;

/*
 * Update the file data.
 */
static void ttt_update(void)
{
	char state[16];

	if (ttt_turn == TTT_OVER)
		strncpy(state, "game over", sizeof(state));
	else if (ttt_turn == TTT_NONE)
		snprintf(state, sizeof(state), "winner is %c", 
			 ttt_game[TTT_WINNER]);
	else
		snprintf(state, sizeof(state), "turn is %c", ttt_turn);

	ttt_output_size = snprintf(ttt_output, TTT_OUTPUT_LENGTH,
			"%c %c %c\n%c %c %c\n%c %c %c\n%s\n",
			ttt_game[1], ttt_game[2], ttt_game[3],
			ttt_game[4], ttt_game[5], ttt_game[6],
			ttt_game[7], ttt_game[8], ttt_game[9],
			state);

	if (ttt_output_size > TTT_OUTPUT_LENGTH)
		printk(KERN_ALERT "TTT output buffer overflow\n");
}

/*
 * Called when the file is read from
 * file    -- ?
 * buffer  OUT the buffer to write into
 * bufsize IN the size of the write buffer
 * offset  IN the offset in the output data to start at
 *         OUT the next position in the output data to start from
 * return: the number of bytes written to the buffer
 *
 */
static ssize_t ttt_proc_read(struct file *file, char *buffer, size_t bufsize,
		loff_t *offset)
{
	int nbytes;

	if (*offset > ttt_output_size)
		return 0;
	nbytes = ttt_output_size - *offset;
	if (nbytes > bufsize)
		nbytes = bufsize;

	printk(KERN_INFO "TTT READ bytes to copy %d\n", nbytes);
	if (copy_to_user(buffer, ttt_output + *offset, nbytes)) {
		printk(KERN_ALERT "TTT READ copy_to_user failed\n");
		return -EFAULT;
	}

	*offset += nbytes;
	return nbytes;
}

/*
 * Called when the file is written to
 */
static ssize_t ttt_proc_write(struct file *file, const char *buffer,
		size_t bufsize, loff_t *offset)
{
	char input[16] = { 0 };
	int nbytes;

	if (*offset > bufsize)
		return 0;

	if (bufsize > sizeof(input) + *offset)
		nbytes = sizeof(input);
	else
		nbytes = bufsize - *offset;

	if (copy_from_user(input, buffer + *offset, nbytes)) {
		printk(KERN_ALERT "TTT WRITE copy_from_user failed\n");
		return -EFAULT;
	}
	input[nbytes] = 0;

	printk(KERN_INFO "TTT WRITE [%d] %s", nbytes, input);

	if (strncmp(input, "reset", 5) == 0) {
		ttt_reset();
		ttt_update();
	} else if (bufsize >= 2) {
		char player = toupper(input[0]);
		int position = input[1] - '0';
		ttt_take_turn(player, position);
		ttt_update();
	}

	*offset += nbytes;
	return nbytes;
}

static struct file_operations ttt_file_ops = {
	.owner = THIS_MODULE,
	.read = ttt_proc_read,
	.write = ttt_proc_write
};

/*
 * Allow other kernel modules to play TTT by submitting turns.
 *   player -- X or O
 *   position -- in 1 to 9, inclusive
 * Return: 0 on success, 1 on failure.
 */
static int ttt_set_turn(char player, int position)
{
	int result = 1;
	player = toupper(player);
	position--;
	if (player == TTT_O || player == TTT_X) {
		ttt_take_turn(player, position);
		ttt_update();
		result = 0; /* SUCCESS */
	}
	return result;
}

/*
 * Get the symbol of the next player's turn, O or X.
 * A '+' indicates the game is over.
 * Return: 0 on success, 1 on failure.
 */
static int ttt_get_turn(char *player)
{
	int result = 1;
	if (player) {
		*player = ttt_turn;
		result = 0; /* SUCCESS */
	}
	return result;
}

/*
 * Get the symbol of the winner, 'O' or 'X'.
 * If the game is not over
 *
 * Return: 0 on success, 1 on failure.
 */
static int ttt_get_winner(char *winner)
{
	int result = 1;
	if (winner && ttt_game[0]) {
		*winner = ttt_game[0];
		result = 0; /* SUCCESS */
	}
	return result;
}

EXPORT_SYMBOL(ttt_get_turn);
EXPORT_SYMBOL(ttt_set_turn);
EXPORT_SYMBOL(ttt_get_winner);

/*
 * Initialize the kernel module.
 */
static __init int ttt_init( void )
{
	struct proc_dir_entry *parent = NULL;
	ttt_proc = proc_create(TTT_PROC_NAME, TTT_MODE, parent, &ttt_file_ops);
	if (!ttt_proc) {
		printk(KERN_ALERT "TTT failed to create proc file\n" );
		return -ENOMEM;
	}

	ttt_reset();
	ttt_update();

	/* Log success */
	printk(KERN_ALERT "TTT loaded\n" );
	return 0;
}

/*
 * Clean up the module.
 */
static __exit void ttt_exit( void )
{
	if (ttt_proc)
		remove_proc_entry(TTT_PROC_NAME, NULL);

	printk(KERN_ALERT "TTT unloaded\n" );
}

module_init(ttt_init);
module_exit(ttt_exit);

/* vim: set ts=8 */

