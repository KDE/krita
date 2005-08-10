/* Routines for manipulating wet pixels.

   Copyright 1999 Raph Levien <raph@gimp.org>

   Released under GPL.

   A wet pixel is an eight word sequence, representing partially
   transparent wet paint on a paper surface.

*/

typedef unsigned char byte;
typedef unsigned short u16;
typedef unsigned int u32;

typedef struct _WetPix WetPix;
typedef struct _WetLayer WetLayer;
typedef struct _WetPack WetPack;

typedef struct _WetPixDbl WetPixDbl;

/* White is made up of myth-red, myth-green, and myth-blue. Myth-red
   looks red when viewed reflectively, but cyan when viewed
   transmissively (thus, it vaguely resembles a dichroic
   filter). Myth-red over black is red, and myth-red over white is
   white.

   Total red channel concentration is myth-red concentration plus
   cyan concentration.

*/

struct _WetPix {
    u16 rd;            /*  Total red channel concentration */
    u16 rw;            /*  Myth-red concentration */
    u16 gd;            /*  Total green channel concentration */
    u16 gw;            /*  Myth-green concentration */
    u16 bd;            /*  Total blue channel concentration */
    u16 bw;            /*  Myth-blue concentration */
    u16 w;            /*  Water volume */
    u16 h;            /*  Height of paper surface */
};

struct _WetLayer {
    WetPix *buf;
    int width;
    int height;
    int rowstride;
};

struct _WetPack {
    int n_layers;
    WetLayer **layers;
};

struct _WetPixDbl {
    double rd;        /*  Total red channel concentration */
    double rw;        /*  Myth-red concentration */
    double gd;        /*  Total green channel concentration */
    double gw;        /*  Myth-green concentration */
    double bd;        /*  Total blue channel concentration */
    double bw;        /*  Myth-blue concentration */
    double w;        /*  Water volume */
    double h;        /*  Height of paper surface */
};

void wet_composite(byte * rgb, int rgb_rowstride,
           WetPix * wet, int wet_rowstride,
           int width, int height);

void wet_composite_layer(byte * rgb, int rgb_rowstride,
             WetLayer * layer,
             int x0, int y0, int width, int height);

void wet_pack_render(byte * rgb, int rgb_rowstride,
             WetPack * pack,
             int x0, int y0, int width, int height);

WetLayer *wet_layer_new(int width, int height);

void wet_layer_clear(WetLayer * layer);

WetPack *wet_pack_new(int width, int height);

void wet_pix_to_double(WetPixDbl * dst, WetPix * src);

void wet_pix_from_double(WetPix * dst, WetPixDbl * src);
