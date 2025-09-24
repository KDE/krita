PRAGMA foreign_keys=off;

-- Create a new table with the constraints set
CREATE TABLE metadata_new (
    id INTEGER PRIMARY KEY
,   foreign_id INTEGER
,   key TEXT
,   value TEXT
,   table_name TEXT
,   UNIQUE(table_name, foreign_id, key)
);

-- Move all the content into the new table
-- WARNING: according to sqlite's docs we must not
--          try to rename the original table, since
--          it may break indexes and other linked 
--          objects
INSERT INTO metadata_new SELECT * FROM metadata;
DROP TABLE metadata;
ALTER TABLE metadata_new RENAME TO metadata;

PRAGMA foreign_keys=on;
