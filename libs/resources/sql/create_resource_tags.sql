CREATE TABLE IF NOT EXISTS resource_tags (
    id INTEGER PRIMARY KEY
,   resource_id INTEGER NOT NULL
,   tag_id INTEGER NOT NULL
,   FOREIGN KEY(resource_id) REFERENCES resources(id)
,   FOREIGN KEY(tag_id) REFERENCES tags(id)
);
