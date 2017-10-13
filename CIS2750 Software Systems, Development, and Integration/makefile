CC = gcc
CFLAGS = -Wall -std=c11 -fPIC `pkg-config --cflags python3`
CPPFLAGS = 
LDFLAGS = 

all: caltool cal.so 

cal.so: calutil.o iCalModule.o
	$(CC) -shared $(LDFLAGS) calutil.o iCalModule.o -o cal.so

iCalModule.o: iCalModule.c calutil.h
	$(CC) $(CFLAGS) iCalModule.c -c 

caltool: calutil.o caltool.o
	$(CC) $(CFLAGS) -o caltool calutil.o caltool.o

calutil.o: calutil.c calutil.h
	$(CC) $(CFLAGS) -c calutil.c 
	
caltool.o: caltool.c caltool.h
	$(CC) $(CFLAGS) -c caltool.c 
	
test: calutil.o write.c
	$(CC) -Wall -std=c11 -o test calutil.o write.c
	
clean:
	rm -f *.o caltool cal.so
	

