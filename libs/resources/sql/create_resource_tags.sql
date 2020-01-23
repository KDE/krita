CREATE TABLE IF NOT EXISTS resource_tags (
    id INTEGER PRIMARY KEY
,   resource_id NOT NULL INTEGER
,   tag_id NOT NULL INTEGER
,   FOREIGN KEY(resource_id) REFERENCES resources(id)
,   FOREIGN KEY(tag_id) REFERENCES tags(id)
);
