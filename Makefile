_CXX_OPTS=-Wall -Werror -O2 -std=c++17 -g $(CXX_OPTS)

OBJS=main.o Logger.o Server.o Response.o Request.o Parser.o StandardHandlers.o

.PHONY: all, clean

all: servd client

clean:
	rm -rf *.o servd

servd: $(OBJS)
	g++ -o servd $(OBJS) -lpthread

%.o: %.cpp
	g++ $(_CXX_OPTS) -c $<

client: client.cpp
	g++ -std=c++14 -o client client.cpp

