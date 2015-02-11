CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS =		confclient.o

LIBS =

TARGET =	confclient

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
