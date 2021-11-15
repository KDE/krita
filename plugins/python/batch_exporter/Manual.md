# Batch Exporter: Krita Plugin for Game Developers and Graphic Designers

Free Krita plugin for designers, game artists and digital artists to work more
productively:

- Batch export assets to multiple sizes, file types, and custom paths. Supports
  `jpg` and `png`.
- Rename layers quickly with the smart rename tool

## Batch Export Layers

Batch Exporter exports individual layers to image files based on metadata in
the layer name. The supported options are:

- `[e=jpg,png]` - supported export image extensions
- `[s=20,50,100,150]` - size in `%`
- `[p=path/to/custom/export/directory]` - custom output path.
  Paths can be absolute or relative to the Krita document.
- `[m=20,30,100]` - extra margin in `px`. The layer is trimmed to the
  smallest bounding box by default. This option adds extra padding around the
  layer.
- `[t=false]` or `[t=no]` - disable trimming the exported layer to the bounding box of
  the content.
- `[i=false]` or `[i=no]` - disable parent metadata inheritance for a layer. More info [below](#layer-inheritance).

A typical layer name with metadata looks like: `CharacterTorso e=png m=30 s=50,100`. This exports
the layer as two images, with an added padding of 30 pixels on each side:
`CharacterTorso_s100_m030.png`, and `CharacterTorso_s050_m030.png`, a copy of the layer scaled down
to half the original size.

All the metadata tags are optional. Each tag can contain one or multiple options
separated by comma `,`. Write `e=jpg` to export the layer to `jpg` only and
`e=jpg,png` to export the layer twice, as a `jpg` and as a `png` file. Note that
the other tag, `p=` has been left out. Below we describe how the plugin works.

## Getting Started

Batch Exporter gives two options to batch export layers: `Export All Layers`
or `Export Selected Layers`.

`Export All Layers` only takes layers with the `e=extension[s]` tag into account. For example, if
the layer name is `LeftArm e=png s=50,100`, `Export All Layers` will take it into account. If the
layer name is `LeftArm s=50,100`, it will not be exported with this option.

`Export Selected Layers` exports all selected layers regardless of the tags.

By default, the plugin exports the images in an `export` folder next to your
Krita document. The export follows the structure of your layer stack. The group
layers become directories and other layers export as files.

> **Supported layer types:** paint, vector, group & file layers.

## Smart Layer Rename tool

Say we have this Krita document structure:

```
GodetteGroupLayer
  +-- HeadGroupLayer
    +-- Hair
    +-- Eyes
    +-- Rest
  +-- Torso
  +-- LeftArm
  +-- RightArm
Background
```

If you want to export `GodetteGroupLayer`, `HeadGroupLayer`, `Torso`, `LeftArm`,
and `RightArm`, but not the other layers, you can select these layers and write
the following in the `Update Layer Name` text box: `e=png s=40,100` and press
<kbd>Enter</kbd>. In this example, Art Tools will export two copies of the
selected layers to png at `40%` and `100%` scale. This is what `s=40,100` does.

Say that we made a mistake: we want to export to `50%` instead of `40%`. Select
the layers once more and write `s=50,100` in the text box. Press
<kbd>Enter</kbd>. This will update the size tag and leave `e=png` untouched.

The tool can do more than add and update meta tags. If you want to remove
`GroupLayer` from the name on `GodetteGroupLayer` and `HeadGroupLayer`, select them
and write `GroupLayer=` in the text box. Press <kbd>Enter</kbd> and the
`GroupLayer` text will disappear from the selected layers.

The `=` tells the tool to search and replace. `this=[that]` will replace `this`
with `[that]`. If you don't write anything after the equal sign, the tool will
erase the text you searched for.

The rename tool is smarter with meta tags. Writing `e=` will remove the
extension tag entirely. For example, `Godete e=png s=50,100` will become
`Godette s=50,100`.

## COA Tools format

The exporter will generate the necessary sprite contents and metadata file for
easy import in COA Tools / Blender.

If you want to export your krita document to COA Tools format,
simply click the `Document` button under COA Tools.

If you want to export multiple or specific COA Tool documents from one Krita document
(if you have e.g. multiple characters in one Krita document),
you can do so by selecting a Group Layer to serve as root for each COA Tool export
you want done.

### Example

You want to export two characters from the same Krita document in one go

```
Root
  +-- Robot (Group Layer)       <-- Select this layer
  |    +-- Head
  |    +-- Body
  |    +-- Legs
  |
  +-- Sketches
  |    +-- ...
  |
  +-- Minion (Group Layer)      <-- ... and this layer
  |    +-- Hat
  |    +-- Head
  |
  Background
```

Once the Group Layers are selected you push "COA Tools -> Selected Layers".

Each export root supports the following metadata:

- `[p=path/to/custom/export/directory]` - custom output path.
  Paths can be absolute or relative to the Krita document.

Each child node of an export root supports the following metadata:

- `[e=jpg,png]` - supported export image extensions

Generating frames to a sprite sheet from a Group Layer is also possible.
Simply mark the layer containing each frame you want in the sheet with a
`c=sheet` and each child layer will act as one frame you can switch when
Working with COA Tools in Blender.

### Example

You want to export a character from the document, and be
able to switch between each state of e.g. the mouth:

```
Root
  +-- Robot (Group Layer)         <-- If this is the export root
  |    +-- Mouth States c=sheet   <-- ... mark this layer
  |    |    +-- Open
  |    |    +-- Half Open
  |    |    +-- Closed
  |    |
  |    +-- Head
  |    +-- Body
  |    +-- Legs
  |
  Background
```

## Layer Inheritance

Batch Exporter now allows child layers to inherit metadata from parent layers
without the `e=` tag. This makes it easier to manage documents with lots of layers
and results in cleaner looking layer names.

Any layers tagged with `i=no` or `i=false` will not inherit metadata from their parent
layers. Tagged group layers will still share **their own** metadata with their children.

### Example

Consider the following document structure:

```
Background e=png m=5 s=50,100 p=assets/images

InterfaceGroupLayer
  +-- ui_skin e=png m=5 s=50,100 p=assets/images/interface
  +-- ui_skin_dark e=png m=5 s=50,100 p=assets/images/interface

MapsGroupLayer
  +-- map01 e=png p=assets/images/interface/maps
  +-- map02 e=png p=assets/images/interface/maps

MobsGroupLayer
  +-- mob01 e=png,jpg m=10 s=75 p=assets/images/mobs
  +-- mob02 e=png,jpg m=10 s=25 p=assets/images/mobs
```

Using metadata inheritance, you could achieve the above like so:

```
InterfaceGroupLayer m=5 s=50,100 p=assets/images/interface
  +-- ui_skin e=png
  +-- ui_skin_dark e=png
  +-- Background e=png p=assets/images

MapsGroupLayer p=assets/images/interface/maps
  +-- map01 e=png
  +-- map02 e=png

MobsGroupLayer p=assets/images/mobs m=10
  +-- mob01 e=png,jpg s=75
  +-- mob02 e=png,jpg s=25
```
