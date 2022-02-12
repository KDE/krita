# Amyspark's YCbCr ICC profiles

This project enables the creation of ITU-R BT.601-7 and BT.709-6 ICC
profiles.

## Characteristics

-   Full-range, floating point Y, Cb, and Cr channels
-   Supports both the BT.601/709 OETF as well as the BT.1886 EOTF curve
-   v2 and v4 profiles
-   For the v4 profiles, a `DtoB0` tag is included that packs the
    complete pipeline in floating point precision (see caveat below)

## Limitations

-   The v2 profiles pack the complete transformation into a single CLUT,
    due to tag limitations
-   The v4 profiles pack the YCbCr \<-\> RGB step into a CLUT for the
    same reason; the remaining steps are explicitly saved into the AtoB0
    components
-   The BT.601/709 curve must be sampled prior to storage in `DtoB0`
    because its parametric shape is not supported by the tag

## Source 

Please visit <https://github.com/amyspark/ycbcr-icc-profiles>.

## License

See [LICENSE-PROFILES.txt](/LICENSE-PROFILES.txt).
