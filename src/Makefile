.SUFFIXES:
.SUFFIXES: .cc .o

CPP = g++
CPPFLAGS = -O3

.cc.o: $*.h
	$(CPP) $(CPPFLAGS) -c $*.cc


OBJECTS = PDICHeader.o PDICIndex.o PDICDatablock.o PDICDatafield.o Criteria.o \
          bsearch.o util.o util_stl.o timeutil.o dump.o utf8.o bocu1.o filemem.o

TEST_OBJECTS = utf8_gtest.o PDICIndex_gtest.o filemem_gtest.o
TEST_DATA = filemem_gtest.dat
TEST_OUTPUTS = filemem_gtest.savemem

pdic: main.o $(OBJECTS)
	$(CPP) $(CPPFLAGS) -o pdic main.o $(OBJECTS) -liconv

pdic_gtest: gtest_main.o $(OBJECTS) $(TEST_OBJECTS) $(TEST_DATA)
	$(CPP) $(CPPFLAGS) -o pdic_gtest gtest_main.o $(OBJECTS) $(TEST_OBJECTS) -lgtest -liconv

run: pdic
	./pdic

test: pdic_gtest
	./pdic_gtest
	rm -f $(TEST_OUTPUTS)

clean:
	rm -f $(TARGET) gtest *.o *~

Criteria.o: Criteria.cc Criteria.h   PDICDatafield.h  bocu1.h util.h dump.h charcode.h
PDICDatablock.o: PDICDatablock.cc PDICDatablock.h  PDICIndex.h Criteria.h  PDICHeader.h PDICDatafield.h util.h bocu1.h charcode.h
PDICDatafield.o: PDICDatafield.cc PDICDatafield.h  util.h bocu1.h utf8.h charcode.h
PDICHeader.o: PDICHeader.cc PDICHeader.h  util.h  dump.h
PDICIndex.o: PDICIndex.cc PDICIndex.h  PDICHeader.h Criteria.h  PDICIndex.h PDICDatablock.h dump.h bocu1.h bsearch.h
bocu1.o: bocu1.cc bocu1.h  utf8.h  dump.h
bsearch.o: bsearch.cc bsearch.h   util.h
dump.o: dump.cc dump.h  util.h  bocu1.h
filemem.o: filemem.cc filemem.h  util_stl.h
timeutil.o: timeutil.cc timeutil.h
utf8.o: utf8.cc utf8.h
util.o: util.cc util.h
util_stl.o: util_stl.cc util_stl.h

main.o: main.cc PDICHeader.h PDICIndex.h PDICDatablock.h PDICDatafield.h util.h util_stl.h bocu1.h dump.h charcode.h timeutil.h bsearch.h cout.h

filemem_gtest.o: filemem_gtest.cc filemem.h dump.h
utf8_gtest.o: utf8_gtest.cc utf8.h
