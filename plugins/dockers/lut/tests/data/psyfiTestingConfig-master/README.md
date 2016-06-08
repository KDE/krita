# psyfiTestingConfig

## What it Is
This is a simple Blender OpenColorIO configuration and LUT / matrix set that is useful for testing color management.

## What it Does
Traditionally, Blender assumes sRGB primaries for input and output. However, some areas remain without color management that form critical pinch points where the primaries or transfer curve cannot be correctly displayed. This configuration shifts simply rotates the sRGB primaries to make it extremely clear where color management is not working.

## Usage
Within your Blender build or install, move the ```<BLENDER_HOME>/bin/<VERSION.NUMBER>/datafiles/colormanagement``` directory to an alternate name to preserve it. Symbollically link this repository in its place.

## Testing
The repository includes a marcie-whacked.exr image that has data values with strongly shifted primaries. There is a default transform that corrects this for pure D65 sRGB output.

## Examples of Current Color Management Breaking Points

### Color Wheel
Currently the color wheel is not correctly displayed. It is impossible to display sRGB on a display, as the color wheel is not hooked into the color management system and cannot be adapted for the intensity / transfer curve and primaries of the display. This is especially noticeable on wide gamut displays.

#### Color Wheel Testing
Change to a paint mode and try painting on the sample image. The colors selected in the wheel will not match the working space.

### Eye Dropper Tool
The eye dropper does not currently convert the sampled working space values to a corrected display output.

#### Eye Dropper Testing
Change to the eye dropper and select an area from the sample image. The results will be displayed untransformed to the artist.

### Multiple Monitor Configurations
Due to the Blender only having a single output display transform, it is impossible to properly display color on multiple head systems. Instead, only one can be properly corrected and hook a profile into the system.

#### Multiple Monitor Testing
Currently unable to demonstrate this shortcoming using this repository. Only a single output transform can be defined, and as such, multiple displays cannot have their profiled adjustments included into the system.


