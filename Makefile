_CXX_OPTS=-std=c++14 -g $(CXX_OPTS)

OBJS=main.o Logger.o Server.o Request.o Parser.o

.PHONY: all, clean

all: servd client

clean:
	rm -rf *.o servd

servd: $(OBJS)
	g++ -o servd $(OBJS)

%.o: %.cpp
	g++ $(_CXX_OPTS) -c $<

client: client.cpp
	g++ -std=c++14 -o client client.cpp

