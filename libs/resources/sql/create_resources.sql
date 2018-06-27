CREATE TABLE IF NOT EXISTS resources (
    id INTEGER PRIMARY KEY
,   resource_type_id INTEGER
,   name TEXT
,   filename TEXT
,   tooltip TEXT
,   thumbnail BLOB
,   status INTEGER
,   FOREIGN KEY(resource_type_id) REFERENCES resource_types(id)
);
