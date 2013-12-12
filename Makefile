OBJS=time_machine.o simclist.o
CFLAGS=-fPIC -Wall
SHARED_LIB=libtimeout.so
LDFLAGS=-ltimeout -L. -lpthread
all: shared test

shared: $(OBJS)
	$(CC) -shared -o $(SHARED_LIB) $(OBJS) $(CFLAGS)

test: test.o
	$(CC) -o tester.exe test.o $(CFLAGS) $(LDFLAGS)

clean:
	rm *.o *.so *.exe -rf
