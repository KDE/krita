CREATE TABLE IF NOT EXISTS storages (
    id INTEGER PRIMARY KEY
,   origin_type_id INTEGER
,   name TEXT
,   location TEXT
,   datestamp TEXT
,   checksum TEXT
,   pre_installed INTEGER
,   active INTEGER
,   FOREIGN KEY(origin_type_id) REFERENCES origin_types(id)
);
