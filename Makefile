all: src/pdic

src/pdic:
	cd src ; make pdic ; cd ..

shell: src/pdic
	src/pdic

test:
	cd src ; make test ; cd ..

clean:
	cd src ; make clean ; cd ..

install: src/pdic
	cp src/pdic /usr/local/bin/

