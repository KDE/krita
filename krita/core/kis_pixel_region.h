#if !defined KIS_PIXEL_REGION_H_
#define KIS_PIXEL_REGION_H_

#include <ksharedptr.h>

class KisPixelPacket;
class KisPixelRegion;

typedef KSharedPtr<KisPixelRegion> KisPixelRegionSP;

// Keep geometry in here
// provide a map to tile pixel
// if an unmodified tile is modified, then copy it
// maybe receive command history?

class KisPixelRegion : public KShared {
public:
	KisPixelPacket* start() { return 0; }
	const KisPixelPacket* start() const { return 0; }
};

#endif // KIS_PIXEL_REGION_H_
