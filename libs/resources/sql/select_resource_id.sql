SELECT resources.id
FROM   resources
,      resource_types
,      storages
WHERE  resources.resource_type_id = resource_types.id
AND    storages.id = resources.storage_id
AND    storages.location = :storage_location
AND    resource_types.name = :resource_type
AND    resources.name = :name
