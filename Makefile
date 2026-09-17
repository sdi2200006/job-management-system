OBJS = jms_coord.o jms_console.o ipc.o handlers.o pool.o
SOURCE = src/jms_coord.c src/jms_console.c src/ipc.c src/handlers.c src/pool.c
HEADER = include/ipc.h include/handlers.h include/pool.h include/shared.h
OUT1 = jms_coord
OUT2 = jms_console
CC = gcc
FLAGS = -g -Wall -c -Iinclude

all: $(OUT1) $(OUT2)

# compile
$(OUT1): jms_coord.o ipc.o handlers.o pool.o
	$(CC) -g jms_coord.o ipc.o handlers.o pool.o -o $(OUT1)

$(OUT2): jms_console.o ipc.o
	$(CC) -g jms_console.o ipc.o -o $(OUT2)

handlers.o: src/handlers.c include/handlers.h include/ipc.h include/pool.h include/shared.h
	$(CC) $(FLAGS) src/handlers.c
	
jms_coord.o: src/jms_coord.c include/handlers.h include/ipc.h include/pool.h include/shared.h
	$(CC) $(FLAGS) src/jms_coord.c

jms_console.o: src/jms_console.c include/ipc.h
	$(CC) $(FLAGS) src/jms_console.c

pool.o: src/pool.c include/pool.h include/shared.h include/handlers.h
	$(CC) $(FLAGS) src/pool.c

ipc.o: src/ipc.c include/ipc.h
	$(CC) $(FLAGS) src/ipc.c
# clean 
clean:
	rm -f $(OBJS) $(OUT1) $(OUT2)