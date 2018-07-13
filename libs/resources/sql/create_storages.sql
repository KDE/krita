CREATE TABLE IF NOT EXISTS storages (
    id INTEGER PRIMARY KEY
,   origin_type_id INTEGER
,   location TEXT
,   timestamp INTEGER
,   pre_installed INTEGER
,   active INTEGER
,   FOREIGN KEY(origin_type_id) REFERENCES origin_types(id)
);
