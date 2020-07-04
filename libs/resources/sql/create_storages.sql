CREATE TABLE IF NOT EXISTS storages (
    id INTEGER PRIMARY KEY AUTOINCREMENT
,   storage_type_id INTEGER
,   location TEXT
,   timestamp INTEGER
,   pre_installed INTEGER
,   active INTEGER
,   thumbnail BLOB           /* the image representing the storage visually*/
,   FOREIGN KEY(storage_type_id) REFERENCES storage_types(id)
,   UNIQUE(location)
);
