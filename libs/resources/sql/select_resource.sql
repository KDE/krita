SELECT resources.*
,      versioned_resources.*
FROM   resources OUTER JOIN versioned_resources
WHERE  resources.id = versioned_resources.resource_id
AND    versioned_resources.version =
    (SELECT MAX(version) FROM versioned_resources
     WHERE  versioned_resources.resource_id = resources.id)
AND    versioned_resources.storage_id =
    (SELECT storage_id FROM storages
     WHERE  location = :storage)
AND    versioned_resources.location = :storage;
