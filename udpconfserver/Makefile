CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS =		confserver.o

LIBS =

TARGET =	confserver

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
