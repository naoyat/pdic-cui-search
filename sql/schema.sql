pragma encoding=utf8;

CREATE TABLE entries (
    id         int PRIMARY KEY,
    entry      varchar(255),
    level      int           default NULL,
    pron       varchar(255)  default NULL
);

CREATE INDEX entries_entry on entries(entry);


CREATE TABLE surfaces (
    surface    varchar(255),
    entry_id   int
);

CREATE INDEX surfaces_surface on surfaces(surface);


CREATE TABLE definitions (
    entry_id   int,
    sub_id     int,
    wordclass  varchar(40),
    item_id    int,
    value      varchar(255)
);

CREATE INDEX definitions_entry_id on definitions(entry_id);


CREATE TABLE aliases (
    from_id    int,
    to_phrase  varchar(255),
    to_id      int
);

CREATE INDEX aliases_from_id on aliases(from_id);
