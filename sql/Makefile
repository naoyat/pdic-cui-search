.SUFFIXES:
.SUFFIXES: .tab .db

.tab.db:
	rm -f $@
	sqlite3 $@ < schema.sql
#	awk -f make-import.awk -v name=$< > $<.import.sql
	awk -f separate.awk -v name=$< $<
	sqlite3 $@ < $<.import.sql

#all: EIJI-135.db WAEI-135.db REIJI135.db RYAKU135.db

all: EIJI-135.db waei-136-tag.db reiji136-tag.db RYAKU135.db

# EIJI-135.db: EIJI-135.sql
