
PEER_TARGET = peer
COMMON_INC = -I ../common

PEER_SOURCE := $(wildcard ../peer/*.cpp) $(wildcard ../common/*.cpp)
PEER_OBJS := $(patsubst %.cpp, %.o, $(PEER_SOURCE))

CXXFLAGS := -g -Wall -DDEBUG $(COMMON_INC)

all:$(PEER_TARGET)

$(PEER_TARGET): $(PEER_OBJS)
	g++ -o $(PEER_TARGET) $(PEER_OBJS) -l pthread -lghthash

clean:
	rm -rf $(PEER_OBJS) $(PEER_TARGET) 

