TARGET = pdic

OBJECTS = PDICHeader.o PDICIndex.o PDICDatablock.o Criteria.o util.o bocu1.o main.o

all: $(OBJECTS)
	g++ -o $(TARGET) $(OBJECTS)


ej: $(TARGET) jiro131u/EIJI-131.DIC
	./$(TARGET) jiro131u/EIJI-131.DIC

wj: $(TARGET) jiro131u/WAEI-131.DIC
	./$(TARGET) jiro131u/WAEI-131.DIC

rj: $(TARGET) jiro131u/REIJI131.DIC
	./$(TARGET) jiro131u/REIJI131.DIC

ry: $(TARGET) jiro131u/RYAKU131.DIC
	./$(TARGET) jiro131u/RYAKU131.DIC


main.o: main.cc # PDICHeader.h PDICIndex.h
	g++ -c main.cc

util.o: util.cc util.h
	g++ -c util.cc

bocu1.o: bocu1.cc bocu1.h
	g++ -c bocu1.cc

PDICHeader.o: PDICHeader.cc PDICHeader.h #util.h bocu1.h
	g++ -c PDICHeader.cc

PDICIndex.o: PDICIndex.cc PDICIndex.h PDICHeader.h #util.h bocu1.h
	g++ -c PDICIndex.cc

PDICDatablock.o: PDICDatablock.cc PDICDatablock.h #PDICHeader.h util.h bocu1.h
	g++ -c PDICDatablock.cc

Criteria.o: Criteria.cc Criteria.h
	g++ -c Criteria.cc

clean:
	rm -f $(TARGET) $(OBJECTS) *~
