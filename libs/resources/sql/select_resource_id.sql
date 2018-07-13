SELECT id
FROM   resources
WHERE  resource_type_id = (SELECT resource_type_id
                           FROM   resource_types
                           WHERE  name = :resource_type)
AND    filename = :filename;
