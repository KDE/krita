CREATE TABLE IF NOT EXISTS tags (
    id INTEGER PRIMARY KEY
,   url TEXT
,   name TEXT
,   comment TEXT
,   resource_type_id INTEGER
,   active INTEGER
,   timestamp INTEGER
,   FOREIGN KEY(resource_type_id) REFERENCES resource_types(id)
,   UNIQUE (url, resource_type_id)
);
