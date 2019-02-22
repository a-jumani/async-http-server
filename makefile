server: server.cpp
	g++ server.cpp -lboost_system -lpthread -lboost_thread -Wall -std=c++11 -I ./include -o server 
clean:
	rm server
