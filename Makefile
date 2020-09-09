_CXX_OPTS=-std=c++14 -g $(CXX_OPTS)

OBJS=main.o Logger.o Server.o Request.o Parser.o

.PHONY: all, clean

all: server client

clean:
	rm -rf *.o servd

server: $(OBJS)
	g++ -o servd $(OBJS)

%.o: %.cpp
	g++ $(_CXX_OPTS) -c $<
	# g++ -std=c++14 -g -c $<

client: client.cpp
	g++ -std=c++14 -o client client.cpp

