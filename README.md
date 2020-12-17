# tty-dayday
Countdown days Since and Until. Draw days using ncurses.
=============================================================

I need a __simple__ program to get the countdown days of important events on my Linux desktop.
ex: How many days for the New Year? How many days for bonus shares to be credited to my account?
But I cannot find a program which is lightweight and simple. So I write tty-dayday.

Run the tty-dayday program on the console. Specify the name of the event (through the '-e' option)
and the date of the event (through the '-d' option).

Example 1:
% > tty-dayday -e 'Working for a living' -d 10/13/2003
![name-of-you-image](https://raw.githubusercontent.com/hwangcc23/tty-dayday/main/screenshot-tty-dayday.png)

Example 2:
% > tty-dayday -e 'New Year 2021' -d 12/31/2020 -u -t 1
![name-of-you-image](https://raw.githubusercontent.com/hwangcc23/tty-dayday/main/screenshot-tty-dayday2.png)
