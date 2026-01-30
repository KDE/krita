# Linked/embedded resources design

In Krita some resources can depend/reuse some other resources. For example a
paintop preset may link to a pattern to be used as a stroke texture. Resources
usually address each other with a "resource signature".

__Resource signature__ (`KoResourceSignature`) is a signature of a resource
that is used for addressing a resource. It consists of multiple fields:

* resource type
* md5sum
* file name
* name

The global database may have multiple resources with the same signature
(given that they are stored in different storages). In such a case the
database code will automatically deduplicate them.

> **_NOTE:_** while the database as a whole might have multiple resources with the
same signature, all resources in the same storage (e.g. folder) must be unique
in regards to (resource-type, file-name) pair. These pairs are used for
addressing inside a single storage, so they cannot duplicate.

## Types of resource dependencies

1) __Linked resources__ (`KoResource::linkedResources()`) is a set of resources
   that a KoResource-object will use internally for its work. Linked resources do
   **not** have a copy inside KoResource-object, so they must be loaded into
   the global database separately **before** using the KoResource-object.
   `KoResource::linkedResources()` is merely a declaration of what an object
   needs.

2) __Embedded resources__ (`KoResource::embeddedResources()`) is a set of resources
    that are stored inside a KoResource-object permanently. When a KoResource-object
    wants to use an embedded resource, it uses the global database for caching
    only. If an embedded resource is not found in the database, then an internal
    copy is loaded.

    Loading an embedded copy of the resource may be slow for big
    resources, e.g. patterns. That is why all the embedded resources are fetched
    into the database on loading a KoResource-object. After that, no internal
    loading is supposed to happen.

    > **_WARNING:_** it is **not recommended** to use embedded resources strategy
    when designing new resource type. Given that the embedded resources are loaded
    into the database on loading a KoResource-object, there is no need to keep
    storing them in memory. It just wastes memory. Use side-loaded resources
    approach instead.

    Embedded resources strategy is used only for the old versions of .kpp format,
    which stores embedded resources explicitly in `KisPropertiesConfiguration`
    object and cannot unload them.

3) __Side-loaded resources__ (`KoResource::sideLoadedResources()`) is a set of
    resources that were loaded alongside the KoResource-object itself, but were
    not connected to anything. Usually they are loaded inside
    `KoResource::loadFromDevice()` and stored in a separate vector inside the
    KoResource-object. When a KoResource-object is then loaded into the database,  `KoResourceLocator` takes all these resources via
    `KoResource::takeSideLoadedResources()` and loads them into the database as well,
    so that they will be accessible to the KoResource-object via `KoResourcesInterface`.

## Abstract resources interface

When a KoResource-object wants to fetch some resource resource from the database
it should use an abstract resources interface object (`KisResourcesInterfaceSP`).

Its usage usually looks like that:

```cpp
auto source = resourcesInterface->source<KoPatternSP>(signature.type);

/**
 * Find the best matching resource using a relaxed strategy. First, it tires
 * to find a resource using the provided md5sum. If nothing is found, it searches
 * for resources with matching name and/or filename
 */
KoPatternSP matchingResource =
    source.bestMatch(signature.md5sum, signature.filename, signature.name);

/**
 * Works like bestMatch(), but will **not** try to match name and filename if
 * md5sum was provided and not found.
 */
KoPatternSP exactResource =
    source.exactMatch(signature.md5sum, signature.filename, signature.name);
```

It has two functions, `bestMatch()` and `exactMatch()` with two strategies of
looking for resources. The rule of thumb is: if you have a backup plan of
loading the resource you need (e.g. when you check if the resource is already
loaded in `KoResource::sideLoadedResources()`), then use `exactMatch()`.
Otherwise use `bestMatch()`.

The global resources database is available to KoResource-objects via
`KisGlobalResourcesInterface::instance()`. Please take it into account
that the global interface **cannot be used from non-gui threads**.

## Local resources snapshots

The global resources database (and its corresponding interface
`KisGlobalResourcesInterface::instance()`) is not available from non-GUI
threads. But most of Krita resources, like paintop presets, are normally
used in non-GUI worker threads when painting, therefore they cannot
access the global cache.

Here comes the abstract nature of the resources interface
(`KisResourcesInterfaceSP`). Before passing a KoResource-object into
a worker thread Krita performs the following:

1) Fetches all the required resources from the database using
    `KoResource::requiredResources()` (which is basically a union
    of linked and embedded resources).
2) Uploads them into an instance of `KisLocalStrokeResources` class,
    which is basically an implementation of `KisResourcesInterface`
    wrapped around a simple `QList` storing all the resources.
3) Clones the KoResource-object and sets its "referred resources
    interface" to the newly created resources snapshot.

All these three steps are performed by a single
`resource->cloneWithResourcesSnapshot()` call.

This ensures that the KoResource-object doesn't have to request
anything from the global database while being used in a non-GUI
thread. That is also the main reason why
`KoResource::linkedResources()` should be accurate about what
resources the object will use.

## Canvas resources interface

Some resources, like gradients, may depend on the currently active
__canvas resources__, like current foreground and background colors.
For this purpose we have an abstract interface
`KoCanvasResourcesInterfaceSP`, which provides access to these
__canvas resources__.

A KoResource-object will declare the required canvas resources
via `KoResource::requiredCanvasResources()`. These canvas resources
will be baked into the resource when it is being cloned.

See implementations of `KoAbstractGradient::cloneAndBakeVariableColors()`
and `KoAbstractGradient::cloneAndUpdateVariableColors()` for more details.


## Preferred process for loading embedded resources

1) Load the resource itself inside `KoResource::loadFromDevice()`
2) Load all the embedded resources and store them as side-loaded
    resources. Basically, implement two methods:

    * `KoResource::sideLoadedResources()`
    * `KoResource::clearSideLoadedResources()`

    Inside these methods use `exactMatch()` for checking for the
    presence of the resources in the database.

3) Depending on the target destination of the resource, either

    * load it into the database using `KisResourceModel::addResource()`,
        then the resource locator will automatically load the side-loaded
        resources.

    * load it into the layer (e.g. a layer style or a pattern generator)
        with manually moving all the side-loaded, embedded and linked
        resources into a local resources snapshot

## UIX considerations for embedded resources

From the UIX point of view, our resources can be split into two groups:

1) __Immutable resources__ are the resources which cannot be changed
    directly using the Krita GUI. The only thing that a user can do
    with them is "select a resource from a list". A good example of
    an immutable resource is a pattern (`KoPattern`).

    > **_NOTE:_** such resources can still be "modified" by loading
    another resource with the same filename using the Resources Manager.
    But we don't consider this action as "modification" in the context
    of UIX discussion, because when overwriting the user explicitly requests
    to upload this resource into the database, in contrast to the case
    when the user just modifies a mutable resource (see below)

    When modifying an embedded immutable resource, the GUI should just
    provide a simple selector of the resources present in the database
    (something based on `KisResourceItemChooser`). When the action is
    finished, the parent KoResource-object will just use the signature
    of the selected resource, which would link to the resource in the
    database.

2) __Mutable resources__ are the resources which **can be edited**
    directly in the Krita GUI. An example of a mutable resource
    is a gradient.

    When some dialog provides a possibility for editing an embedded
    mutable resource, the GUI should be a little bit more complex.
    The GUI should **not** directly edit resources in the database.
    Instead, the GUI should provide controls showing the state
    of "the current resource" (which is not stored anywhere). And this
    state can either be loaded from an existing resource in the database
    or saved into the database. An example of such a GUI is
    `KisGenericGradientEditor`.

    This separate notion of "the current resource" is needed, because
    the user might change the resource (say, a gradient) a hundred of
    times before he/she gets satisfied with it.

### Reference algorithm for loading a resource with linked/embedded resources into GUI

1) Create a temporary memory-based resource storage for the embedded immutable
    resources and for the original state of immutable resources. Use
    `KisTemporaryResourceStorageLock` to do than, then the storage will be
    automatically deleted when the dialog is closed.

2) If the resource has a local resources snapshot (`KisLocalStrokeResources`), upload
    all unknown resources from this snapshot into the temporary memory storage
    from the previous step.

    * Use `exactMatch()` matching strategy for verifying if the resource is already
        present or not (remember, if a resource with exactly the same md5sum is not
        found, we have a backup plan, we can upload it from `KisLocalStrokeResources`)

    * When adding to the storage, make sure to use a deduplicated version of `addResource()`
        method (i.e. `KisResourceModel::addResourceDeduplicateFileName()`). It may happen
        that multiple embedded resources have the same filename, but different md5sum. We
        need to make sure that both the resources end up being in the storage without
        overwriting each other.

    > **_NOTE:_** Loading of the embedded resources into a temporary storage will
    let selectors of immutable resources show these resources in the list

3) If the resource has no local resources snapshot, create one and assign it
    to the resource. That will make the resource self-contained.

    * Use `KisRequiredResourcesOperators::createLocalResourcesSnapshot()` to do that

    * Do **not** bake canvas resources (via `KoCanvasResourcesInterfaceSP`), the baking
        should happen only when we assign the resource to a layer or pass it into a
        worker thread.

    > **_NOTE:_** Later, this local resources snapshot will let us store
        "the current mutable embedded resource" (i.e. gradient) directly in the
        instance of `KisLocalStrokeResources` without uploading it into the
        global database.

4) Pass the loaded resource to the GUI controls.

    * Selectors for immutable resources will **not** use this local snapshot.
        Instead, they will use the signature of the embedded resource to fetch
        a copy of it from the temporary memory storage.

    * GUI for mutable resources will use the local resources snapshot to fetch
        the embedded resource and use it as "the current resource". It is needed
        because after the first modification, the embedded resource will no longer
        be present in the database.

### Reference algorithm for saving a resource with linked/embedded resources from GUI

1) Create a new instance of `KisLocalStrokeResources`, which will store all the embedded
    resources.

2) Ask the GUI controls to save all the embedded resources into this
    `KisLocalStrokeResources`-object.

3) After saving, assign this `KisLocalStrokeResources`-object as a snapshot
    for the resource via `resource->setResourcesInterface(newLocalResources)`

Now we have a self-contained resource, which stores all the required resources
in a local resources snapshot. It can be assigned to layers directly via simple
`resource->cloneWithResourcesSnapshot()`.

If the user requests to save this self-contained resource into the database,
we will have to do a few additional steps to make sure the resources from the
snapshot are also loaded into the database.

1) Move all the resources from the local snapshot into side-loaded resources
    vector (see `KisAslLayerStyleSerializer::sideLoadLinkedResources`)

2) Reset the resources interface of the object to
    `KisGlobalResourcesInterface::instance()`.

3) Pass the resulting resource into `KisResourceModel::addResource()`. The resource
    locator will automatically load all the side-loaded resources into the storage.

> **_WARNING:_** side-loaded resources operate with objects of type `KoEmbeddedResource`,
    not with normal `KoResource` objects. It means they should be serialized into a `QBuffer`
    first, which might theoretically change their md5sum. Be careful with that!






