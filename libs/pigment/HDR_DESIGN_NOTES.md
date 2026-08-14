# Krita HDR Design Notes

## Basic terminology

1. “HDR Reference White” aka “Graphics White” aka “Diffuse White” is a point in an HDR space to which we connect the brightest point of an SDR space. Basically, when converting scene-referred rec2020-g10 into display-referred rec2020-pq, source point of (1.0,1.0,1.0) will be mapped to this selected lightness level in PQ space.

    As per ITU-R BT.2408-8, “HDR Reference White” is a level of brightness as the nominal signal level obtained from an HDR camera and a 100% reflectance white card

2. “Display Diffuse White Level” aka “SDR peak white”  is the brightness of the diffuse white point on a particular display. The value is selected by the user in the HDR settings of the compositor. When Krita passes data to the compositor for rendering, it should map the “diffuse white” point of the image color space to the value selected by the used.

## CRWL tag

1. Only PQ profiles can have CRWL tag set
2. Krita provides two kinds of PQ profiles, one for HDR Reference White set to 80 nits (legacy) and another for 203 nits (new)
3. The user should be able to do "Assign profile" on the image with a different value of "HDR Reference White". For backwards compatibility
4. Scene-referred profiles do not have this tag in the CMS. It is just an attribute in .kra
5. When rendering a scene-referred space into Wayland’s PQ space, the user-selected HDR Reference White (in .kra metadata) level is **not** used. A value of 1.0 is directly mapped to the level requested by the compositor
6. Backwards compatibility:
    1. When loading a .kra file with the fake `ITUR_2100_PQ_FULL` profile is loaded as 80 nits
    2. When loaded other file formats (e.g. .png or .tiff) with `ITUR_2100_PQ_FULL` profile, then 203 nits profiles is used

## cICP tags in ICC

1. For all runtime-generated profiles the cICP tag is set, given that the color space can be represented with cICP
    Which usually includes the following:
    1. All profiles generated to pass data to Wayland (usually they are **not** saved to disk)
    2. Both Rec2020PQ profiles (80nits and 203nits)
2. Which, effectively means that we “save” cICP tag on disk only for Rec2020PQ profiles
3. TODO: do we need to autogenerate Rec2020PQ profiles or we could just pregenerate and save them once? It would resolve at least one problem deduplicating of deduplicating the embedded profiles on loading.

## cICP saving in .kra

1. When saving to .kra
    * if the profile is Rec2020PQ, it has cICP data
2. When loading from .kra
    * Load the PQ profile normally and connect it to the CMS (unless it has the same name/properties). Loaded cICP values will be preserved.

## cICP saving in .png

1. PNG specification declares that cICP values has higher priority over the embedded profile
2. When saving to .png
    * Save cICP into the PNG directly (also mDCV and cLLI if present)
    * Embed the profile normally, i.e. only PQ profiles will have cICP values
3. When loading from .png
    * Check if cICP tag can be loaded directly from PNG. If yes, reuse our own profile. Skip loading the embedded profile.
    * If not present, load the embedded ICC profile
        * Load the profile normally and connect it to the engine. cICP values will be used **only** to detect Rec2020PQ profile
        * if the profile in Rec2020PQ and has CRWL tag, use it while connecting to the engine
        * if profile name equals `ITUR_2100_PQ_FULL`, reuse existing Rec2020PQ profile. Skip loading the embedded profile.
            * NOTE: the 203 nits version of the profile should be used

## HDR metadata saving

1. When saving to .kra
    * Save HDR metadata into .kra
    * If the profile is Rec2020PQ, it already has CRWL tag declaring its HDR Reference White Level
2. When loading from .kra
    * Load HDR metadata from .kra (warn if CRWL data in the ICC is inconsistent with the one in .kra)
    * If the color space is “scene referred”, then CRWL tag of the ICC profile is **not** used in any way. Only the metadata from KisImage is used
    * if the color space  is Rec2020PQ, then CRWL tag of the ICC profile is used. The metadata from KisImage is **not** used
3. Automatic setting of the "Diffuse White" level metadata
    * When the image color space is converted into a display-referred
      profile with a CRWL tag, the "Diffuse White" metadata is
      automatically synced with CRWL tag.
    * When an image is converted back from a display-referred profile
      into a scene-referred profile, the "Diffuse White" metadata
      is kept intact. The reason is that resetting this metadata
      would make relative content light level information invalid.
    * When and HDR image with HDR metadata is converted into
      non-HDR space, then HDR metadata is removed.

## Links

* To store the color space luminance we need CRWL value from here: [https://registry.color.org/dicttype-metadata/](https://registry.color.org/dicttype-metadata/)
* HDR PNG specification: [https://www.w3.org/TR/png-3/](https://www.w3.org/TR/png-3/)