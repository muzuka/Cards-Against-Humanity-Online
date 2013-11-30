all: Server.o Client.o Player.o Card.o
	g++ Card.o Player.o Client.o -o Client
	g++ Card.o Player.o Server.o -o Server

Server.o: Server.cpp
	g++ -c Server.cpp

Client.o: Client.cpp
	g++ -c Client.cpp
	
Player.o: Player.cpp
	g++ -c Player.cpp

Card.o: Card.cpp
	g++ -c Card.cpp

clean: 
	rm *.o
