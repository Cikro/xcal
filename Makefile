CC = gcc
CFLAGS = -Wall -std=c11 -fPIC `pkg-config --cflags python3` 
LDFlags = 
all: 
	make caltool
	make cal.so

caltool: caltool.o calutil.o
	$(CC) $(CFLAGS) -g $^ -o $@

calutil.o: calutil.c calutil.h
	$(CC) $(CFLAGS) -g $< -c -o $@ 
	
caltool.o: caltool.c caltool.h
	$(CC) $(CFLAGS) -g $< -c -o $@ 

cal.so: calmodule.o calutil.o caltool.o
	$(CC) $(CFLAGS) -shared $^ $(LDFlags) -o $@

calmodule.o: calModule.c calutil.h caltool.h
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -rf testcal caltool *.o cal.so __pycache__