CC    = g++ 
CXX   = g++
SERVERDIR=.
DEPENDS=../depends/
WEBSOCKET=$(DEPENDS)include/websocketpp

INC=-I$(SERVERDIR)/ -Icommon -Imain -Itask -I$(WEBSOCKET) -I./ -I/usr/local/boost/include/ -I/usr/local/include -I$(DEPENDS)include/zookeeper -I$(DEPENDS)/include

LIB= -L$(DEPENDS)/lib -L/usr/local/lib/ -L/usr/local/boost/lib /usr/local/lib/libjson.a  /usr/local/boost/lib/libboost_chrono.a -lboost_system

CFLAGS= -w  -g  -std=c++11 


TARGET= libcommon.a

SRCEXT  := .cpp 
SRCDIRS :=$(patsubst ./%,%,$(shell find . -type d))
SRCLIST :=$(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*, $(SRCEXT))))
OBJS 	:= $(patsubst %.cpp,%.o,$(SRCLIST))

SRCEXT  := .cc
SRCLIST :=$(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*, $(SRCEXT))))
OBJS   += $(patsubst %.cc,%.o,$(SRCLIST))


all: $(TARGET)
	
$(TARGET):$(OBJS)
	ar rcs $@ $^ 

checklink:$(OBJS)
	$(CXX) $(CFLAGS) -o $@ $^ $(LIB)
	
%.o: %.cpp
	$(CXX) $(CFLAGS) $(INC) -c -o $@ $<
%.o: %.c
	$(CXX) $(CFLAGS) $(INC) -c -o $@ $<
%.o: %.cc
	$(CXX) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	rm  $(OBJS)
	rm -f $(TARGET)
