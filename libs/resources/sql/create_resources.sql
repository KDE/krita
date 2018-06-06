CREATE TABLE IF NOT EXISTS resources (
    id INTEGER PRIMARY KEY
,   resource_type_id INTEGER
,   name TEXT
,   tooltip TEXT
,   thumbnail BLOB
,   FOREIGN KEY(resource_type_id) REFERENCES resource_types(id)
);
