CREATE TABLE IF NOT EXISTS resources (
    id INTEGER PRIMARY KEY   /* within this database, a unique and stable id for this resource */
,   resource_type_id INTEGER /* points to the type of this resource */
,   storage_id INTEGER       /* points to the storage object that contains the actual resource */
,   name TEXT NOT NULL       /* the untranslatable name of the resource */
,   filename TEXT NOT NULL   /* the filename of the resource RELATIVE to the storage path */
,   tooltip TEXT             /* a translated text that can be shown in the UI */
,   thumbnail BLOB           /* the image representing the resource visually*/
,   status INTEGER           /* active resources are visible in the UI, inactive resources are considered "deleted" */
,   FOREIGN KEY(resource_type_id) REFERENCES resource_types(id)
,   UNIQUE(storage_id, resource_type_id, name)
,   UNIQUE(storage_id, filename)
);
