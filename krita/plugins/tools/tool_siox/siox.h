/**
 *  Copyright 2005, 2006 by Gerald Friedland, Kristian Jantz and Lars Knipping
 *
 *  Conversion to C++ for Inkscape by Bob Jamison
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * Note by Bob Jamison:
 * After translating the siox.org Java API to C++ and receiving an
 * education into this wonderful code,  I began again,
 * and started this version using lessons learned.  This version is
 * an attempt to provide an dependency-free SIOX engine that anyone
 * can use in their project with minimal effort.
 *
 * Many thanks to the fine people at siox.org.
 */

#ifndef SIOX_H
#define SIOX_H

#include <map>
#include <string>
#include <vector>
#include "kis_types.h"
#include "cielab.h"
#include "classresult.h"
#include "blob.h"

class KisSelectionManager;
class KisCanvasSubject;

enum SioxRefinementType
{
    SIOX_REFINEMENT_NO_CHANGE          = 0,
    SIOX_REFINEMENT_ADD_FOREGROUND     = (1 << 0),
    SIOX_REFINEMENT_ADD_BACKGROUND     = (1 << 1),
    SIOX_REFINEMENT_CHANGE_SENSITIVITY = (1 << 2),
    SIOX_REFINEMENT_CHANGE_SMOOTHNESS  = (1 << 3),
    SIOX_REFINEMENT_CHANGE_MULTIBLOB   = (1 << 4),
    SIOX_REFINEMENT_RECALCULATE        = 0xFF
};

class Siox {
public:
    Siox(KisCanvasSubject* subject);
    /**
    * siox_foreground_extract:
    * @state:       current state struct as constructed by siox_init
    * @refinement:  #SioxRefinementType
    * @mask:        a mask indicating sure foreground (255), sure background (0)
    *               and undecided regions ([1..254]).
    * @x1:          region of interest
    * @y1:          region of interest
    * @x2:          region of interest
    * @y2:          region of interest
    * @sensitivity: a double array with three entries specifing the accuracy,
    *               a good value is: { 0.64, 1.28, 2.56 }
    * @smoothness:  boundary smoothness (a good value is 3)
    * @multiblob:   allow multiple blobs (true) or only one (false)
    *
    * Writes the resulting segmentation into @mask. The region of
    * interest as specified using @x1, @y1, @x2 and @y2 defines the
    * bounding box of the background and undecided areas. No changes to
    * the mask are done outside this rectangle.
    */
    void foregroundExtract(SioxRefinementType refinement, Q_INT32 x1, Q_INT32 y1, Q_INT32 x2, Q_INT32 y2, Q_INT32 smoothness, const double sensitivity[3], bool multiblob);
private:
    /**
     *  Stage 1 of the color signature work.  'dims' will be either
     *  2 for grays, or 3 for colors
     */
    void colorSignatureStage1(CieLab *points,
                              unsigned int leftBase,
                              unsigned int rightBase,
                              unsigned int recursionDepth,
                              unsigned int *clusters,
                              const unsigned int dims);

    /**
     *  Stage 2 of the color signature work
     */
    void colorSignatureStage2(CieLab         *points,
                              unsigned int leftBase,
                              unsigned int rightBase,
                              unsigned int recursionDepth,
                              unsigned int *clusters,
                              const float  threshold,
                              const unsigned int dims);
    /**
     *  Main color signature method
     */
    bool colorSignature(const std::vector<CieLab> &inputVec,
                        std::vector<CieLab> &result,
                        const unsigned int dims);
    /**
     * Squared Euclidian distance of p and q.
     */
    float sqrEuclidianDist(float *p, int pSize, float *q);
    float sqrEuclidianDist(CieLab& p, CieLab& q);
    /* This method checks out the neighbourhood of the pixel at position
    * (x,y) in the TileManager mask, it adds the surrounding pixels to
    * the queue to allow further processing it uses maskVal to determine
    * if the surrounding pixels have already been visited x,y are passed
    * from above.
    */
    void depthFirstSearch(Q_INT32 x, Q_INT32 y, Q_UINT32 xwidth, Q_UINT32 yheight, Blob* b, Q_UINT8 mark);
    /* Digitize mask */
    void thresholdMask (Q_INT32 x, Q_INT32 y, Q_UINT32 width, Q_UINT32 height);
    /* Smoothes mask by delegation to paint-funcs.c */
    void smoothMask (Q_INT32 x, Q_INT32 y, Q_UINT32 width, Q_UINT32 height);
    /* Erodes mask by delegation to paint-funcs.c */
    void erodeMask (Q_INT32 x, Q_INT32 y, Q_UINT32 width, Q_UINT32 height);
    /* Dilates mask by delegation to paint-funcs.c */
    void dilateMask (Q_INT32 x, Q_INT32 y, Q_UINT32 width, Q_UINT32 height);
    /*
    * This method finds the biggest connected components in mask, it
    * clears everything in mask except the biggest components' Pixels that
    * should be considererd set in incoming mask, must fulfill (pixel &
    * 0x1) the method uses no further memory, except a queue, it finds
    * the biggest components by a 2 phase algorithm 1. in the first phase
    * the coordinates of an element of the biggest components are
    * identified, during this phase all pixels are visited. In the
    * second phase first visitation flags are reset, and afterwards
    * connected components starting at the found coordinates are
    * determined. These are the biggest components, the result is written
    * into mask, all pixels that belong to the biggest components are set
    * to 255, any other to 0.
    */
    void findMaxBlob (Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height, const Q_INT32 sizeFactor);
    /* Returns squared clustersize */
    float getClustersize (const float *limits);
    bool cacheRemoveBg (Classresult& value);
    bool cacheRemoveFg (Classresult& value);
    /* Creates a key for the hashtable from a given pixel color value */
    int createKey (const unsigned char *src, int bpp);
    unsigned long getRGB(int a, int r, int g, int b);
    void drb(int x, int y, int brushRadius, int brushMode, float threshold);
private:
    /**
     * Our signature limits
     */
    float limits[3];
    KisCanvasSubject* m_subject;
    KisPaintDeviceSP m_dev;
    KisPaintDeviceSP m_mask;
    KisSelectionManager* m_selectionManager;
    Q_INT32 m_x;
    Q_INT32 m_y;
    Q_INT32 m_width;
    Q_INT32 m_height;
    std::map<int, Classresult> m_cache;
    std::vector<CieLab> m_bgSig;
    std::vector<CieLab> m_fgSig;
};

#endif //SIOX_H
