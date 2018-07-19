CREATE TABLE IF NOT EXISTS tags (
    id INTEGER PRIMARY KEY
,   resource_type_id INTEGER
,   url TEXT
,   name TEXT
,   comment TEXT
,   active INTEGER
,   FOREIGN KEY(resource_type_id) REFERENCES resource_types(id)
,   UNIQUE (url, resource_type_id)
);
