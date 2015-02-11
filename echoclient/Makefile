CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS =		echoclient.o

LIBS =

TARGET =	echoclient

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
