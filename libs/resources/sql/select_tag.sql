SELECT tags.id
FROM   tags
,      resource_types
WHERE  tags.url = :url
AND    resource_types.id = tags.resource_type_id
AND    resource_types.name = :resource_type;
