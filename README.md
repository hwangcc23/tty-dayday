# tty-dayday
Countdown days Since and Until. Draw days using ncurses.
=============================================================

[About]:
I need a __simple__ program to get the countdown days of important events on my Linux desktop.
ex: How many days for the New Year? How many days for bonus shares to be credited to my account?
But I cannot find a program which is lightweight and simple. So I write tty-dayday.

[Usage]:
tty-dayday is a console program using ncurses.
Run the program on the console. Specify the name of the event (through the '-e' option)
and the date of the event (through the '-d' option).

$> tty-dayday
Usage: tty-dayday -e NAME_OF_EVENT -d DATE_IN_MM/DD/YYYY [options]
Options:
  -v|--version      Print the version number and exit
  -h|--help         Print the help messages and exit

$> tty-dayday -e 'Working for a living' -d 10/13/2003

![name-of-you-image](https://raw.githubusercontent.com/hwangcc23/tty-dayday/main/screenshot-tty-dayday.png)
