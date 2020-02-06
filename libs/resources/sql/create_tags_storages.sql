CREATE TABLE IF NOT EXISTS tags_storages (
    tag_id INTEGER
,   storage_id INTEGER
,   FOREIGN KEY(storage_id) REFERENCES storages(id)
,   FOREIGN KEY(tag_id) REFERENCES tags (id)
,   UNIQUE (tag_id, storage_id)
);
