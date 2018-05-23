CREATE TABLE IF NOT EXISTS stores (
    id INTEGER PRIMARY KEY,
    origin_type_id INTEGER,
    FOREIGN KEY(origin_type_id) REFERENCES origin_types(id)
)
