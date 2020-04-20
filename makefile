CXX = g++
CXXFLAGS = -std=c++11 -pthread -Wall -I dependencies/asio/asio/include/ -I dependencies/cxxopts/include/

out/ping: out/main.o out/pinger.o out/echo_packet.o
	$(CXX) $(CXXFLAGS) out/main.o out/pinger.o out/echo_packet.o -o out/ping

out/main.o: src/main.cc src/pinger.h
	$(CXX) $(CXXFLAGS) -c src/main.cc -o out/main.o

out/pinger.o: src/pinger.cc src/pinger.h
	$(CXX) $(CXXFLAGS) -c src/pinger.cc -o out/pinger.o

out/echo_packet.o: src/echo_packet.cc src/echo_packet.h
	$(CXX) $(CXXFLAGS) -c src/echo_packet.cc -o out/echo_packet.o

clean:
	rm out/*.o out/ping