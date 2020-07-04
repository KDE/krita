CREATE TABLE IF NOT EXISTS resource_metadata (
    id INTEGER PRIMARY KEY
,   resource_id INTEGER
,   key TEXT
,   value TEXT
,   FOREIGN KEY(resource_id) REFERENCES resources(id)
);


INSERT INTO storage_types
VALUES name = "Memory";


UPDATE TABLE version_information
SET    database_version = "0.0.3"
