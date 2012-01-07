CPP = g++
CPPFLAGS = -O2

TARGET = pdic
OBJECTS = PDICHeader.o PDICIndex.o PDICDatablock.o Criteria.o util.o util_stl.o bocu1.o main.o

$(TARGET): $(OBJECTS)
	$(CPP) $(CPPFLAGS) -o $(TARGET) $(OBJECTS) -liconv


run: $(TARGET)
	./$(TARGET)


main.o: main.cc # PDICHeader.h PDICIndex.h
	$(CPP) $(CPPFLAGS) -c main.cc

util.o: util.cc util.h
	$(CPP) $(CPPFLAGS) -c util.cc

util_stl.o: util_stl.cc util.h
	$(CPP) $(CPPFLAGS) -c util_stl.cc

bocu1.o: bocu1.cc bocu1.h
	$(CPP) $(CPPFLAGS) -c bocu1.cc

PDICHeader.o: PDICHeader.cc PDICHeader.h #util.h bocu1.h
	$(CPP) $(CPPFLAGS) -c PDICHeader.cc

PDICIndex.o: PDICIndex.cc PDICIndex.h PDICHeader.h #util.h bocu1.h
	$(CPP) $(CPPFLAGS) -c PDICIndex.cc

PDICDatablock.o: PDICDatablock.cc PDICDatablock.h #PDICHeader.h util.h bocu1.h
	$(CPP) $(CPPFLAGS) -c PDICDatablock.cc

Criteria.o: Criteria.cc Criteria.h
	$(CPP) $(CPPFLAGS) -c Criteria.cc

clean:
	rm -f $(TARGET) $(OBJECTS) *~
