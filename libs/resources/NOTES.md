DONE

 * copy over resources to the user's resources folder
 * initial creation of the database schema
 * initial filling of the database
 * folder storage class
 * tag loading from desktop files`
 * updating the cache db with changes done directly on the resources folder
 * bundle storage class
 * Database explorer ui
 * models for integrating with the ui
 * implement dirty resources mechanism
 * Remove all hints that bundles are resources.
 * implement metadata mechanism for resources --> the KisResourceModel class has many unimplemented methods atm
 * Implement in-memory resources, that are deleted from the db when Krita starts (add to locator cache, set id on resource, add to database, update model)

DOING

 * Implement KoResourceServer as a shim for KoResourceModel
 * Make KisFavoriteManager a model-view class for presets and tags
 * fix loading the actual preset in the preset delegate

TODO

 * asl storage class
 * abr storage class
 * adding/updating/removing resources from the ui
 * adding/removing tags from the ui
 * database migration and versioning (loop over version, find update_from_version, execute and so on until version is up to date)
 * fetch gradients from resource server by md5 or name when handling layer styles: fix how layer styles store gradients
 * fix reloading the preset if dirty presets is switched off (KisPaintopBox::resourceSelected). The problem is that the preset itself no longer knows where it comes from or how it should be loaded
