/* synthesize a surface texture */

void wet_layer_maketexture(WetLayer * layer,
               double height, double blurh, double blurv);

void wet_layer_clone_texture(WetLayer * dst, WetLayer * src);

void wet_pack_maketexture(WetPack * pack,
              double height, double blurh, double blurv);
