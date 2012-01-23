.SUFFIXES:
.SUFFIXES: .cc .o

CXX = g++
CXXFLAGS=-Wall -O3 -g -DDEBUG -DVERBOSE -I.
# CXXFLAGS = -O3 -DDEBUG

.cc.o: $*.h
	$(CXX) $(CXXFLAGS) -c $*.cc

OBJECTS = \
	obj/pdic/PDICHeader.o obj/pdic/PDICIndex.o obj/pdic/PDICDatablock.o \
	obj/pdic/PDICDatafield.o obj/pdic/Criteria.o obj/pdic/Dict.o obj/pdic/lookup.o \
	obj/util/bocu1.o obj/util/dump.o obj/util/filemem.o obj/util/macdic_xml.o \
	obj/util/search.o obj/util/stlutil.o obj/util/timeutil.o obj/util/utf8.o \
	obj/util/util.o obj/util/Shell.o

TEST_OBJECTS = test/filemem_gtest.o test/search_gtest.o \
               test/utf8_gtest.o test/util_gtest.o test/shell_gtest.o
TEST_DATA = test/filemem_gtest.dat
TEST_OUTPUTS = test/filemem_gtest.savemem

pdicsh: main.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o pdicsh main.o $(OBJECTS) -liconv -lre2

pdicsh_re2: main.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o pdicsh_re2 main.o $(OBJECTS) libre2.a -liconv

pdic_gtest: gtest_main.o $(OBJECTS) $(TEST_OBJECTS) $(TEST_DATA)
	$(CXX) $(CXXFLAGS) -o pdic_gtest $(OBJECTS) $(TEST_OBJECTS) -lgtest -lgtest_main -liconv -lre2

EIJIRO_gtest: gtest_main.o $(OBJECTS) EIJIRO_gtest.o
	$(CXX) $(CXXFLAGS) -o EIJIRO_gtest $(OBJECTS) test/EIJIRO_gtest.o -lgtest -lgtest_main -liconv -lre2

debug: pdicsh
	gdb pdicsh

shell: pdicsh
	if [ x`which rlwrap` = x ]; then ./pdicsh ; else rlwrap ./pdicsh ; fi

install: pdicsh
	cp pdicsh /usr/local/bin/

test: pdic_gtest
	./pdic_gtest
	rm -f $(TEST_OUTPUTS)

test_eijiro: EIJIRO_gtest
	./EIJIRO_gtest

clean:
	rm -f pdicsh pdic_gtest *.o $(OBJECTS) *~ $(TEST_OUTPUTS)

lint:
	cpplint.py main.cc util/*.h util/*.cc pdic/*.h pdic/*.cc test/*.cc

obj/%.o: %.cc
	@mkdir -p $$(dirname $@)
	$(CXX) -o $@ $(CXXFLAGS) -c $*.cc
