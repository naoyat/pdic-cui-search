.SUFFIXES:
.SUFFIXES: .cc .o

CPP = g++
CPPFLAGS = -O3 -DDEBUG

.cc.o: $*.h
	$(CPP) $(CPPFLAGS) -c $*.cc


OBJECTS = PDICHeader.o PDICIndex.o PDICDatablock.o PDICDatafield.o Criteria.o Dict.o \
		bocu1.o cout.o dump.o filemem.o search.o shell.o timeutil.o utf8.o util.o util_stl.o

TEST_OBJECTS = PDICIndex_gtest.o filemem_gtest.o search_gtest.o shell_gtest.o utf8_gtest.o util_gtest.o
TEST_DATA = filemem_gtest.dat
TEST_OUTPUTS = filemem_gtest.savemem

pdic: main.o $(OBJECTS)
	rm search.o
	$(CPP) $(CPPFLAGS) -c search.cc
	$(CPP) $(CPPFLAGS) -o pdic main.o $(OBJECTS) -liconv -lre2

pdic_gtest: gtest_main.o $(OBJECTS) $(TEST_OBJECTS) $(TEST_DATA)
	rm search.o
	$(CPP) $(CPPFLAGS) -DTEST -c search.cc
	$(CPP) $(CPPFLAGS) -DTEST -o pdic_gtest gtest_main.o $(OBJECTS) $(TEST_OBJECTS) -lgtest -liconv -lre2

shell: pdic
	./pdic

test: pdic_gtest
	./pdic_gtest
	rm -f $(TEST_OUTPUTS)

clean:
	rm -f pdic pdic_gtest *.o *~

Criteria.o: Criteria.cc Criteria.h  PDICDatafield.h bocu1.h charcode.h dump.h types.h util.h
Dict.o: Dict.cc Dict.h  Criteria.h PDICHeader.h PDICIndex.h PDICDatablock.cc PDICDatafield.h \
        bocu1.h charcode.h filemem.h timeutil.h util_stl.h utf8.h types.h
PDICDatablock.o: PDICDatablock.cc PDICDatablock.h  Criteria.h PDICDatafield.h PDICHeader.h PDICIndex.h bocu1.h charcode.h util.h types.h
PDICDatafield.o: PDICDatafield.cc PDICDatafield.h  bocu1.h charcode.h utf8.h util.h
PDICHeader.o: PDICHeader.cc PDICHeader.h  dump.h util.h
PDICIndex.o: PDICIndex.cc PDICIndex.h  PDICDatablock.h PDICHeader.h PDICIndex.h bocu1.h search.h dump.h
bocu1.o: bocu1.cc bocu1.h  dump.h types.h utf8.h
cout.o: cout.cc cout.h
dump.o: dump.cc dump.h  bocu1.h dump.h types.h util.h
filemem.o: filemem.cc filemem.h  types.h util_stl.h
search.o: search.cc search.h  util.h
shell.o: shell.cc shell.h  Dict.h PDICDatafield.h PDICHeader.h PDICIndex.h timeutil.h util.h util_stl.h types.h
timeutil.o: timeutil.cc timeutil.h
utf8.o: utf8.cc utf8.h  types.h
util.o: util.cc util.h  types.h
util_stl.o: util_stl.cc util_stl.h

main.o: main.cc shell.h
gtest_main.o: gtest_main.cc

filemem_gtest.o: filemem_gtest.cc filemem.h dump.h
search_gtest.o: search_gtest.cc  search.h util.h
shell_gtest.o: shell_gtest.cc  shell.h
utf8_gtest.o: utf8_gtest.cc  types.h utf8.h
util_gtest.o: util_gtest.cc  types.h util.h
