WITH tags_linked_to_storage AS (
    SELECT tags_storages.tag_id
    FROM tags_storages
        INNER JOIN tags ON tags.id = tags_storages.tag_id
        INNER JOIN resource_types ON resource_types.id = tags.resource_type_id
        INNER JOIN storages ON storages.id = tags_storages.storage_id
    WHERE resource_types.name = :resource_type
        AND storages.location = :location)
SELECT tags_storages.tag_id AS tag_id, COUNT(*) AS ref_count
FROM tags_storages
WHERE tags_storages.tag_id IN tags_linked_to_storage
GROUP BY tags_storages.tag_id