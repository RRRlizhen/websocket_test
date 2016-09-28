CC    = g++ 
CXX   = g++

INCLUDE = -I./ -I/usr/local/include/websocket -I/usr/local/include/boost
LIBRARY = -L/usr/local/lib -ljsoncpp -lboost_system -lboost_chrono -lpthread

CFLAGS= -w  -g  -std=c++11 

TARGET= myapp

SRCEXT  := .cpp 
SRCDIRS :=$(patsubst ./%,%,$(shell find . -type d))
SRCLIST :=$(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*, $(SRCEXT))))
OBJS 	:= $(patsubst %.cpp,%.o,$(SRCLIST))

SRCEXT  := .cc
SRCLIST :=$(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*, $(SRCEXT))))
OBJS   += $(patsubst %.cc,%.o,$(SRCLIST))


all: $(TARGET)
	
$(TARGET):$(OBJS)
	$(CXX) $(CFLAGS) -o $@ $^ $(LIBRARY) 

checklink:$(OBJS)
	$(CXX) $(CFLAGS) -o $@ $^ $(LIBRARY)
	
%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -c -o $@ $<
%.o: %.c
	$(CXX) $(CFLAGS) $(INCLUDE) -c -o $@ $<
%.o: %.cc
	$(CXX) $(CFLAGS) $(INCLUDE) -c -o $@ $<

clean:
	rm  $(OBJS)
	rm -f $(TARGET)
