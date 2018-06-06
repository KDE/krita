CREATE TABLE IF NOT EXISTS storages (
    id INTEGER PRIMARY KEY
,   origin_type_id INTEGER
,   name TEXT
,   location TEXT
,   FOREIGN KEY(origin_type_id) REFERENCES origin_types(id)
);
