.SUFFIXES:
.SUFFIXES: .cc .o

CPP = g++
CPPFLAGS = -O3

.cc.o: $*.h
	$(CPP) $(CPPFLAGS) -c $*.cc


OBJECTS = PDICHeader.o PDICIndex.o PDICDatablock.o PDICDatafield.o Criteria.o \
          bsearch.o util.o util_stl.o timeutil.o dump.o utf8.o bocu1.o filemem.o suffix_array.o

TEST_OBJECTS = utf8_gtest.o PDICIndex_gtest.o filemem_gtest.o suffix_array_gtest.o
TEST_DATA = filemem_gtest.dat
TEST_OUTPUTS = filemem_gtest.savemem

pdic: main.o $(OBJECTS)
	$(CPP) $(CPPFLAGS) -o pdic main.o $(OBJECTS) -liconv

pdic_gtest: gtest_main.o $(OBJECTS) $(TEST_OBJECTS) $(TEST_DATA)
	rm suffix_array.o
	$(CPP) $(CPPFLAGS) -DTEST -c suffix_array.cc
	$(CPP) $(CPPFLAGS) -DTEST -o pdic_gtest gtest_main.o $(OBJECTS) $(TEST_OBJECTS) -lgtest -liconv

run: pdic
	./pdic

test: pdic_gtest
	./pdic_gtest
	rm -f $(TEST_OUTPUTS)

clean:
	rm -f $(TARGET) gtest *.o *~

Criteria.o: Criteria.cc Criteria.h  PDICDatafield.h bocu1.h charcode.h dump.h types.h util.h
PDICDatablock.o: PDICDatablock.cc PDICDatablock.h  Criteria.h PDICDatafield.h PDICHeader.h PDICIndex.h bocu1.h charcode.h util.h types.h
PDICDatafield.o: PDICDatafield.cc PDICDatafield.h  bocu1.h charcode.h utf8.h util.h
PDICHeader.o: PDICHeader.cc PDICHeader.h  dump.h util.h
PDICIndex.o: PDICIndex.cc PDICIndex.h  PDICDatablock.h PDICHeader.h PDICIndex.h bocu1.h bsearch.h dump.h
bocu1.o: bocu1.cc bocu1.h  dump.h types.h utf8.h
bsearch.o: bsearch.cc bsearch.h  util.h
dump.o: dump.cc dump.h  bocu1.h dump.h types.h util.h
filemem.o: filemem.cc filemem.h  types.h util_stl.h
suffix_array.o: suffix_array.cc suffix_array.h  util.h
timeutil.o: timeutil.cc timeutil.h
utf8.o: utf8.cc utf8.h  types.h
util.o: util.cc util.h  types.h
util_stl.o: util_stl.cc util_stl.h

main.o: main.cc  Criteria.h PDICDatablock.h PDICDatafield.h PDICHeader.h PDICIndex.h bocu1.h bsearch.h charcode.h cout.h dump.h timeutil.h util.h util_stl.h

filemem_gtest.o: filemem_gtest.cc filemem.h dump.h
utf8_gtest.o: utf8_gtest.cc  types.h utf8.h
suffix_array_gtest.o: suffix_array_gtest.cc  suffix_array.h util.h
