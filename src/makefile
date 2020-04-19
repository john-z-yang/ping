CXX = g++
CXXFLAGS = -std=c++11 -pthread -Wall -I ../dependencies/boost_1_72_0/ -I ../dependencies/cxxopts/include/

ping: main.o pinger.o echo_packet.o
	$(CXX) $(CXXFLAGS) main.o pinger.o echo_packet.o -o ping

main.o: main.cc pinger.h
	$(CXX) $(CXXFLAGS) -c main.cc

pinger.o: pinger.cc pinger.h
	$(CXX) $(CXXFLAGS) -c pinger.cc

echo_packet.o: echo_packet.cc echo_packet.h
	$(CXX) $(CXXFLAGS) -c echo_packet.cc

clean:
	rm *.o ping