CREATE TABLE IF NOT EXISTS tags (
    id INTEGER PRIMARY KEY
,   resource_type_id INTEGER
,   storage_id INTEGER       /* points to the storage object that contains the actual tag */
,   url TEXT
,   name TEXT
,   comment TEXT
,   active INTEGER
,   FOREIGN KEY(resource_type_id) REFERENCES resource_types(id)
,   UNIQUE (url, resource_type_id)
);
