-- Deduplicate metadata entries in the table
DELETE FROM metadata
WHERE id NOT IN (
    SELECT MAX(id)
    FROM metadata
    GROUP BY table_name, foreign_id, key
);

-- Delete all dangling metadata for deleted resources
DELETE FROM metadata
WHERE foreign_id NOT IN (
    SELECT id
    FROM resources
) AND table_name = "resources";

-- Delete all dangling metadata for deleted storages
DELETE FROM metadata
WHERE foreign_id NOT IN (
    SELECT id
    FROM storages
) AND table_name = "storages";
