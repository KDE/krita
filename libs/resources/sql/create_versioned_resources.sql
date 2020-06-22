CREATE TABLE IF NOT EXISTS versioned_resources (
    id INTEGER PRIMARY KEY
,   resource_id INTEGER
,   storage_id INTEGER
,   version INTEGER
,   location TEXT NOT NULL
,   md5sum TEXT NOT NULL
,   timestamp INTEGER
,   FOREIGN KEY(resource_id) REFERENCES resources(id)
,   FOREIGN KEY(storage_id) REFERENCES storages(id)
);
