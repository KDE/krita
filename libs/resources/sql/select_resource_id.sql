SELECT resources.id
FROM   resources
,      resource_types
WHERE  resource_type_id = resource_types.id
AND    resource_types.name = :resource_type
AND    filename = :filename;
