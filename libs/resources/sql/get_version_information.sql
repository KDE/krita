SELECT (
    database_version
,   krita_version
,   creation_date (
FROM version_information
ORDER BY id
DESC
LIMIT 1;
