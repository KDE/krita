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

PLAN

 * Implement KoResourceServer as a shim for KoResourceModel
 * Make KisFavoriteManager a model-view class for presets and tags
 * Remove all hints that bundles are resources.
 * Implement in-memory resources, that are deleted from the db when Krita starts (add to locator cache, set id on resource, add to database, update model)

TODO

 * asl storage class
 * abr storage class
 * adding/updating/removing resources from the ui
 * adding/removing tags from the ui
 * database migration and versioning (loop over version, find update_from_version, execute and so on until version is up to date)
 * fix loading the actual preset in the preset delegate
 * fetch gradients from resource server by md5 or name when handling layer styles: fix how layer styles store gradients

