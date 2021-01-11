CC = gcc
SRC = tty-dayday.c
BINARY = tty-dayday
CFLAGS = -Wall
LDFLAGS = -lm

ifeq ($(shell sh -c 'which ncurses6-config>/dev/null 2>/dev/null && echo y'), y)
	CFLAGS += $$(ncurses6-config --cflags)
	LDFLAGS += $$(ncurses6-config --libs)
else ifeq ($(shell sh -c 'which ncurses5-config>/dev/null 2>/dev/null && echo y'), y)
	CFLAGS += $$(ncurses5-config --cflags)
	LDFLAGS += $$(ncurses5-config --libs)
else
	CFLAGS += $$(pkg-config --cflags ncurses)
	LDFLAGS += $$(pkg-config --libs ncurses)
endif

tty-dayday: ${SRC}
	${CC} ${CFLAGS} ${SRC} -o ${BINARY} ${LDFLAGS}

clean:
	@rm -f ${BINARY}
