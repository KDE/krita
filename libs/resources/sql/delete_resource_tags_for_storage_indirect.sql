WITH storage_id_query
AS (SELECT storages.id
    FROM   storages
    WHERE  storages.location = :location)
DELETE FROM resource_tags
    WHERE resource_id
    IN (SELECT id FROM resources
        WHERE storage_id = (SELECT * FROM storage_id_query))
