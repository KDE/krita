# Tags Design Notes

## Design decisions

1) It is impossible for a tag to be linked to no storage. Any tag should be linked at least to something, e.g. to the _folder_ storage

1) Tags from the folder storage may be saved into the _folder_ storage by the user manually. In this case we should keep all deleted tags in the database to prevent them from appearing in the GUI again and again after restart.

1) Tags are **not** automatically promoted to a "local" tag on resource assignment. I.e. a tag **belong** to the storages it comes from.

1) `tags_storages` table registers "where the tag comes from", so that we could cleanup now-unused tags of the removed storages. There is no one-to-one relation between tags and storages. One tag can come from multiple storages.

1) When multiple storages provide tags with the same URL, all such tags will be merged into one. The `tags_storages` table will have one record for each such storage.

1) [current behavior] The `tags_storages` table works like a reference-countng table. When the **last storage owning a tag** is removed, Krita removes the tag as well, whether or not there are still resources tagged with this tag, i.e.
   * if a tag links the resources from the removed storage only, then the resources and the tag are removed
   * if a tag links the resources from the outside of the removed storage, these external resources will be untagged, and only the tag will be removed

2) [planned behavior] Currently, a tag from one storage can be linked to a resource from another storage, and, when the first storage is removed, the resource from the second storage will be untagged. We plan to change this behavior by indroducing the following rule:

   * if the user manually assigns a tag to a resource from outside of the tag's storage, then the tag becomes "global", i.e. saved in the folder storage

   It will basically mean that all cross-storage tags will be automatically added to the main _folder_ storage, which cannot be removed, which, in turn, guarantees that the tag will never be removed either.

## Requirements

### 1. Tag is added via GUI

* the tag is added into the _folder_ storage
* this type of tags is kept forever, i.e. they can never be deleted from the database, only deactivated

### 2. Tag is removed via GUI

* tag belongs to the _folder_ storage only
  * the tag is deactivated
* tag belongs to the bundle storage only
  * the tag is deactivated
* tag belongs to the _folder_ storage and a bundle as well
  * the tag is deactivated

### 3. Resource is tagged
* resource belongs to the same storage as the tag
  * just tag the resource (add a row in `resource_tags`)
* resource belongs to a different storage than the tag
  * [TO BE IMPLEMENTED] promote the tag to a "local tag" (link it to _folder_ storage in `tags_storages` table)
  * then tag the resource (add a row in `resource_tags`)

### 4. Resource is untagged
* resource belongs to the same storage as the tag
  * just untag (mark the link as "inactive" in `resource_tags` table)
* resource belongs to a different storage than the tag
  * just untag (mark the link as "inactive" in `resource_tags` table)

### 5. Storage is added/sync'ed
* [TO BE IMPLEMENTED] the tags stored in a storage can address **only** the resources from this storage (currently it tries to address the resources globally)
* if two storages provide two tags with the same URL, they should be merged, `tags_storages` table will have two rows, one for each storage

### 6. Storage is removed
* if some other storage still links this tag
  * the tag is preserved
* if the tag links the resources in the removed storage only
  * the tag is removed completely
* if the tag also links resources outside the removed storage
  * [CURRENTLY] all external resources are untagged and the tag is removed
  * [TO BE IMPLEMENTED] the tag is promoted to a local tag and kept in the database
  * [TO BE IMPLEMENTED] ASSERT: if a tag links to resources outside the storage, it **must** also be linked to at least one storage, e.g. _folder_ storage (see the "promote to local on assignment" requirement above)

### 7. Storage is deactivated
* deactivation of a storage implicitly deactivates tags linked to this storage, i.e. a tag is active if at least one of its storages is active
* NOTE: the actual code doing that is placed in `KisTagModel::filterAcceptsRow`

### 8. Storage is reactivated
* reactivation of a storage implicitly activates a tag

## TODO
1) Cleanup/promote orphaned tags on migration
2) Cleanup for orphaned storage-tag records on migration
3) Cleanup for orphaned tag-resource records on migration