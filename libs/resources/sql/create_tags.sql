CREATE TABLE IF NOT EXISTS tags (
    id INTEGER PRIMARY KEY
,   resource_type_id INTEGER
,   url TEXT COLLATE NOCASE  /* the unique untranslated name of the tag */
,   name TEXT                /* the translated name of the tag */
,   comment TEXT             /* a translated comment on the tag */
,   active INTEGER
,   FOREIGN KEY(resource_type_id) REFERENCES resource_types(id)
,   UNIQUE (url, resource_type_id)
);
