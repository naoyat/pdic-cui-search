.SUFFIXES:
.SUFFIXES: .cc .o

CPP = g++
CPPFLAGS = -O2

.cc.o: $*.h
	$(CPP) $(CPPFLAGS) -c $*.cc


OBJECTS = PDICHeader.o PDICIndex.o PDICDatablock.o Criteria.o util.o util_stl.o bocu1.o

TEST_OBJECTS = util_gtest.o util.o \
	PDICIndex_gtest.o

pdic: main.o $(OBJECTS)
	$(CPP) $(CPPFLAGS) -o pdic main.o $(OBJECTS) -liconv

pdic_gtest: gtest_main.o $(TEST_OBJECTS)
	$(CPP) $(CPPFLAGS) -o pdic_gtest gtest_main.o $(TEST_OBJECTS) -lgtest -liconv

run: pdic
	./pdic

test: pdic_gtest
	./pdic_gtest

clean:
	rm -f $(TARGET) gtest *.o *~

util.o: util.cc util.h
util_stl.o: util_stl.cc util.h
bocu1.o: bocu1.cc bocu1.h
PDICHeader.o: PDICHeader.cc PDICHeader.h #util.h bocu1.h
PDICIndex.o: PDICIndex.cc PDICIndex.h PDICHeader.h #util.h bocu1.h
PDICDatablock.o: PDICDatablock.cc PDICDatablock.h #PDICHeader.h util.h bocu1.h
Criteria.o: Criteria.cc Criteria.h
# main.o: main.cc
# gtest_main.o: gtest_main.cc
