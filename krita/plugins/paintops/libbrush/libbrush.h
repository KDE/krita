March 2008

About libbrush 

libbrush is a library for handling brush resources. Brush resources
should not be confused with brush engines (or paintops). Brush engines
may or may not use a brush resource. Basically, brush resources provide
an image to make "footprints" with on the canvas, like with a potato
stamp. 

A brush's task is to basically return an image or a mask (depending on
the type) when the brush engine asks for it. The KisBrush class can
also keep a set of prescaled brush images or masks, which is handy
when using the pressure-size curve option for tablets.

There are several kinds of brush types:

 * gbr: Gimp one-image brushes, either a grayscale mask or a coloured image.
        There are also 16-bit .gbr brushes, created and used by CinePaint. 
        Krita is compatible with those, too.
 * gih: Gimp image hose brushes. Image hoses contain a set of images and
        some instructions to determine which image comes next
 * custom: this brush is generator from the image or a selection of the image
           and can be saved as an ordinary gbr or gih brush.
 * text: a basic brush created from some text and a font
 * auto: a brush defined from a shape and some parameters.

Other applications support other types of brushes:

 * png: krita used to use png images in stead of gbr brushes, and there
        are (march 2009) some noises that GIMP might want to use png
        brushes again.
 * svg: defining brushes in svg format has the advantage that scaling is
        lot nicer.
 * abr: photoshop's brush format. Early versions are open, later versions
        are closed
 * painter: Corel Painter has its own closed brush format that would be 
        interesting to support
 * myb: mypaint brushes. Open source and we really should try to support
        them, though it may only be useful for a special-built paintop.

Right now there is no way to categorize brushes into sets: there is some
GIMP code that allows users to tag resources, but it is not easy to use
that to deliver brush sets that are pre-tagged.

Architecture

The original brush resource architecture used a resource server,
resource mediator and resource chooser. It was impossible to retrieve
a specific brush by name or to serialize and deserialize brushes.
We are transitioning to the ordinary Krita
registry/factory/instance/settings/settings widget  pattern, where
the registry loads plugins, the plugins add a factory to the registry
and provides instances to the application.  For now, it's still a
bit mixed.

Brush plugins cannot yet provide a gui for editing or choosing a brush;
all brush plugins must add the brushes they load to the brush resource
server. The interaction is as follows:

 * KisBrushRegistry is instantiated
 * KBR loads all plugins
 * Plugins instantiate a factory and add it to the KBR
 * The KBR asks the factories to load existing resources
 * The factories add the existing resources to the KisBrushServer

Serializing and deserializing, and creating brushes

Brushes have a toXML function that can be used to store a brush with
settings in an xml file, for instance for action recording or paintop
presets. The xml _must_ contain a brush_type_id. 

The KisBrushRegistry has a getOrCreate method that takes this xml and
determines which bruhs factory to use: the brush factory then analyzes
the xml and determines whether to create a new brush or retrieve an
instance of a loaded brush.

The KisBrush::fromXML method hides all this complexity from the user.
