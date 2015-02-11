CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS =		peer.o

LIBS =

TARGET =	peer

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
