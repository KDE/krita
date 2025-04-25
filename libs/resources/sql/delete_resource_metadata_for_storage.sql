DELETE FROM metadata
    WHERE metadata.table_name = :table
    AND foreign_id IN (SELECT id FROM resources
                       WHERE resources.storage_id =
                            (SELECT storages.id
                             FROM   storages
                             WHERE  storages.location = :location))