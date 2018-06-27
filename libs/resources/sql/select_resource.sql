SELECT resources.id
FROM   resources JOIN versioned_resources
WHERE  resources.id = versioned_resources.resource_id
AND    versioned_resources.version =
    (SELECT MAX(version) FROM versioned_resources
     WHERE  versioned_resources.resource_id = resources.id)
AND    versioned_resources.storage_id =
    (SELECT storage_id FROM storages
     WHERE  location = :storage)
AND    resources.resource_type_id =
    (SELECT resource_type_id FROM resource_types
     WHERE  name = :resource_type)
AND    versioned_resources.location = :location;
