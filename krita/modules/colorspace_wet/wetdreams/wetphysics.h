void wet_pix_combine(WetPixDbl * dst, WetPixDbl * src1, WetPixDbl * src2);

void wet_pix_dilute(WetPixDbl * dst, WetPix * src, double dilution);

void wet_flow(WetLayer * layer);

void wet_dry(WetLayer * layer);

void wet_adsorb(WetLayer * layer, WetLayer * adsorb);
