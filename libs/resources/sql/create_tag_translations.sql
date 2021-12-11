CREATE TABLE IF NOT EXISTS tag_translations (
    id INTEGER PRIMARY KEY
,   tag_id INTEGER
,   language TEXT
,   name TEXT                /* the translated name of the tag */
,   comment TEXT             /* a translated comment on the tag */
,   FOREIGN KEY(tag_id) REFERENCES tags(id)
,   UNIQUE (tag_id, language)
);
