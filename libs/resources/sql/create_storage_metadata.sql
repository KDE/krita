CREATE TABLE IF NOT EXISTS storage_metadata (
    id INTEGER PRIMARY KEY
,   storage_id INTEGER
,   key TEXT
,   value TEXT
,   FOREIGN KEY(storage_id) REFERENCES storages(id)
);
