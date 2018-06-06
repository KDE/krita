CREATE TABLE IF NOT EXISTS resource_tags (
    id INTEGER PRIMARY KEY
,   resource_id INTEGER
,   tag_id INTEGER
,   FOREIGN KEY(resource_id) REFERENCES resources(id)
,   FOREIGN KEY(tag_id) REFERENCES tags(id)
);
