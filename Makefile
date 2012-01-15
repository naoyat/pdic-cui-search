all: src/pdicsh

src/pdicsh:
	cd src ; make pdicsh ; cd ..

shell: src/pdicsh
	src/pdicsh

test:
	cd src ; make test ; cd ..

clean:
	cd src ; make clean ; cd ..

install: src/pdicsh
	cp src/pdicsh /usr/local/bin/

