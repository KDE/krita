CREATE TABLE IF NOT EXISTS resources (
    id INTEGER PRIMARY KEY
,   resource_type_id INTEGER
,   storage_id INTEGER
,   name TEXT NOT NULL
,   filename TEXT NOT NULL
,   tooltip TEXT
,   thumbnail BLOB
,   status INTEGER
,   FOREIGN KEY(resource_type_id) REFERENCES resource_types(id)
,   UNIQUE(storage_id, resource_type_id, name)
,   UNIQUE(storage_id, filename)
);
