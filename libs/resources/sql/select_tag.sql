SELECT id
FROM   tags
WHERE  url = :url
AND    resource_type_id =
    (SELECT resource_type_id
     FROM   resource_types
     WHERE  name = :resource_type);
