= Kernel Tic Tac Toe =

== Build & Install ==
Edit the Makefile to set KDIR to the location of your kernel source tree,
then type

    $ make
    # insmod ttt.ko
    $ lsmod | grep ttt

The module creates a file in the proc file system called "ttt" the reports
the game state when read and sets player's turns when written to.

== Play ==
Its tic-tac-toe with players X and O taking turns until there is a winner
or the board is full.  O goes first.  Write the player's letter followed by
the position, 1 to 9.  Write "reset" to start a new game.

    $ cat /proc/ttt
    $ echo "O5" > /proc/ttt
    $ echo "X3" > /proc/ttt
    ...
    $ cat /proc/ttt
    $ echo "reset" > /proc/ttt

