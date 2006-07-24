/*
   Copyright 2005, 2006 by Gerald Friedland, Kristian Jantz and Lars Knipping

   Conversion to C++ for Inkscape by Bob Jamison

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 */

#include "siox.h"

#include <math.h>
#include <stdarg.h>
#include <map>
#include <stack>
#include <cstdlib>

#include "qrect.h"

#include <kaction.h>
#include "kdebug.h"

#include "kis_iterators_pixel.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_selection.h"
#include "kis_selection_manager.h"
#include "kis_canvas_subject.h"

#define SIOX_COLOR_DIMS 3

#define MIN(a, b) ((a) < (b) ? (a) : (b))

Siox::Siox(KisCanvasSubject* subject) : m_subject(subject) {
    Q_CHECK_PTR(m_subject);

    KisImageSP image = m_subject->currentImg();
    Q_CHECK_PTR(image);

    KisLayerSP src = image->activeLayer();
    Q_CHECK_PTR(src);

    m_dev = image->activeDevice();
    Q_CHECK_PTR(m_dev);

    m_mask = m_dev->selection();
    Q_CHECK_PTR(m_mask);

    m_selectionManager = m_subject->selectionManager();
    Q_CHECK_PTR(m_selectionManager);

    QRect rect = m_dev->exactBounds();
    m_x = rect.x();
    m_y = rect.y();
    m_width = rect.width();
    m_height = rect.height();
}

/**
 *  Stage 1 of the color signature work.  'dims' will be either
 *  2 for grays, or 3 for colors
 */
void Siox::colorSignatureStage1(CieLab *points,
                                unsigned int leftBase,
                                unsigned int rightBase,
                                unsigned int recursionDepth,
                                unsigned int *clusterCount,
                                const unsigned int dims)
{

    unsigned int currentDim = recursionDepth % dims;
    CieLab point = points[leftBase];
    float min = point(currentDim);
    float max = min;

    for (unsigned int i = leftBase + 1; i < rightBase ; i++)
        {
        point = points[i];
        float curval = point(currentDim);
        if (curval < min) min = curval;
        if (curval > max) max = curval;
        }

    //Do the Rubner-rule split (sounds like a dance)
    if (max - min > limits[currentDim])
        {
        float pivotPoint = (min + max) / 2.0; //average
        unsigned int left  = leftBase;
        unsigned int right = rightBase - 1;

        //# partition points according to the dimension
        while (true)
            {
            while ( true )
                {
                point = points[left];
                if (point(currentDim) > pivotPoint)
                    break;
                left++;
                }
            while ( true )
                {
                point = points[right];
                if (point(currentDim) <= pivotPoint)
                    break;
                right--;
                }

            if (left > right)
                break;

            point = points[left];
            points[left] = points[right];
            points[right] = point;

            left++;
            right--;
            }

        //# Recurse and create sub-trees
        colorSignatureStage1(points, leftBase, left,
                 recursionDepth + 1, clusterCount, dims);
        colorSignatureStage1(points, left, rightBase,
                 recursionDepth + 1, clusterCount, dims);
        }
    else
        {
        //create a leaf
        CieLab newpoint;

        newpoint.C = rightBase - leftBase;

        for (; leftBase < rightBase ; leftBase++)
            {
            newpoint.add(points[leftBase]);
            }

        //printf("clusters:%d\n", *clusters);

        if (newpoint.C != 0)
            newpoint.mul(1.0 / (float)newpoint.C);
        points[*clusterCount] = newpoint;
        (*clusterCount)++;
        }
}



/**
 *  Stage 2 of the color signature work
 */
void Siox::colorSignatureStage2(CieLab         *points,
                                unsigned int leftBase,
                                unsigned int rightBase,
                                unsigned int recursionDepth,
                                unsigned int *clusterCount,
                                const float  threshold,
                                const unsigned int dims)
{

  
    unsigned int currentDim = recursionDepth % dims;
    CieLab point = points[leftBase];
    float min = point(currentDim);
    float max = min;

    for (unsigned int i = leftBase+ 1; i < rightBase; i++)
        {
        point = points[i];
        float curval = point(currentDim);
        if (curval < min) min = curval;
        if (curval > max) max = curval;
        }

    //Do the Rubner-rule split (sounds like a dance)
    if (max - min > limits[currentDim])
        {
        float pivotPoint = (min + max) / 2.0; //average
        unsigned int left  = leftBase;
        unsigned int right = rightBase - 1;

        //# partition points according to the dimension
        while (true)
            {
            while ( true )
                {
                point = points[left];
                if (point(currentDim) > pivotPoint)
                    break;
                left++;
                }
            while ( true )
                {
                point = points[right];
                if (point(currentDim) <= pivotPoint)
                    break;
                right--;
                }

            if (left > right)
                break;

            point = points[left];
            points[left] = points[right];
            points[right] = point;

            left++;
            right--;
            }

        //# Recurse and create sub-trees
        colorSignatureStage2(points, leftBase, left,
                 recursionDepth + 1, clusterCount, threshold, dims);
        colorSignatureStage2(points, left, rightBase,
                 recursionDepth + 1, clusterCount, threshold, dims);
        }
    else
        {
        //### Create a leaf
        unsigned int sum = 0;
        for (unsigned int i = leftBase; i < rightBase; i++)
            sum += points[i].C;

        if ((float)sum >= threshold)
            {
            float scale = (float)(rightBase - leftBase);
            CieLab newpoint;

            for (; leftBase < rightBase; leftBase++)
                newpoint.add(points[leftBase]);

            if (scale != 0.0)
                newpoint.mul(1.0 / scale);
            points[*clusterCount] = newpoint;
            (*clusterCount)++;
            }
      }
}

/**
 *  Main color signature method
 */
bool Siox::colorSignature(const std::vector<CieLab> &inputVec,
                          std::vector<CieLab> &result,
                          const unsigned int dims)
{

    unsigned int length = inputVec.size();

    if (length < 1) // no error. just don't do anything
        return true;

    CieLab *input = (CieLab *) malloc(length * sizeof(CieLab));

    if (!input)
        {
        //error("Could not allocate buffer for signature");
        return false;
        }
    for (unsigned int i=0 ; i < length ; i++)
        input[i] = inputVec[i];

    unsigned int stage1length = 0;
    colorSignatureStage1(input, 0, length, 0, &stage1length, dims);

    unsigned int stage2length = 0;
    colorSignatureStage2(input, 0, stage1length, 0, &stage2length, length * 0.001, dims);

    result.clear();
    for (unsigned int i=0 ; i < stage2length ; i++)
        result.push_back(input[i]);

    free(input);

    return true;
}

/**
 * Squared Euclidian distance of p and q.
 */
float Siox::sqrEuclidianDist(float *p, int pSize, float *q)
{
    float sum=0.0;
    for (int i=0; i<pSize; i++)
        {
        float v = p[i] - q[i];
        sum += v*v;
        }
    return sum;
}

float Siox::sqrEuclidianDist(CieLab& p, CieLab& q)
{
    return ((p.L - q.L) * (p.L - q.L)
          + (p.A - q.A) * (p.A - q.A) 
          + (p.B - q.B) * (p.B - q.B));
}

/* Mask settings for threshold_mask
 * Do not change these defines! They contain some magic!
 * Must all be non-zero and FINAL must be 0xFF!
 */
 
#define SIOX_LOW  1
#define SIOX_HIGH 254

#define FIND_BLOB_SELECTED  0x1
#define FIND_BLOB_FORCEFG   0x3
#define FIND_BLOB_VISITED   0x7
#define FIND_BLOB_FINAL     0xFF

//XXXX: something is wrong here. It just fills the whole selection
/* Digitize mask */
void Siox::thresholdMask (Q_INT32 x, Q_INT32 y, Q_UINT32 width, Q_UINT32 height)
{

    //read
    KisRectIteratorPixel it = m_mask->createRectIterator(x, y, width, height, true);
    while( ! it.isDone() ) {
        //depth of mask is one byte
        if (it.rawData()[0] > SIOX_HIGH) it.rawData()[0] = FIND_BLOB_FORCEFG;
        else if (it.rawData()[0] >= 0x80) it.rawData()[0] = FIND_BLOB_SELECTED;
        else it.rawData()[0] = 0;
        ++it;
    }
}

//XXXX: depthFirstSearch hangs, there is something wrong!

/* This method checks out the neighbourhood of the pixel at position
 * (x,y) in the TileManager mask, it adds the surrounding pixels to
 * the queue to allow further processing it uses maskVal to determine
 * if the surrounding pixels have already been visited x,y are passed
 * from above.
 */

void Siox::depthFirstSearch(Q_INT32 x, Q_INT32 y, Q_UINT32 xwidth, Q_UINT32 yheight, Blob* b, Q_UINT8 mark)
{
    std::stack <Q_INT32> s;
    Q_INT32 xx = b->seedx;
    Q_INT32 yy = b->seedy;
    Q_INT32 oldx = -1;

    while (true)
    {
        Q_UINT8 val;

        if (oldx == xx)
        {
            break;
            /*if (s.empty())
                break;
    
            xx = s.top();
            s.pop();
    
            yy = s.top();
            s.pop();*/
            //XXXX: in gimp this is
            /*if (stack == NULL)
                break;

            xx    = GPOINTER_TO_INT (stack->data);
            stack = g_slist_delete_link (stack, stack);
    
            yy    = GPOINTER_TO_INT (stack->data);
            stack = g_slist_delete_link (stack, stack);
            */
            //I think we don't need this because we create our Stack on the stack...
        }

        oldx = xx;

        m_mask->readBytes(&val, xx, yy, 1, 1);

        if (val && (val != mark))
        {
            if (mark == FIND_BLOB_VISITED)
            {
                ++(b->size);
                if (val == FIND_BLOB_FORCEFG) b->mustkeep = true;
            }

            m_mask->readBytes(&mark, xx, yy, 1, 1);

            if (yy > y) {
                s.push(yy - 1);//if it does not work, try it the other way round
                s.push(xx);
            }
            if (yy + 1 < yheight) {
                s.push(yy + 1);
                s.push(xx);
            }
            if (xx + 1 < xwidth) {
                if (xx > x) {
                    s.push(yy);
                    s.push(xx -1);
                }
                ++xx;
            }
            else if (xx > x) {
                --xx;
            }
        }
    }
}

/* Smoothes mask by delegation to paint-funcs.c */
void Siox::smoothMask (Q_INT32 x, Q_INT32 y, Q_UINT32 width, Q_UINT32 height) {
    m_selectionManager->smooth();
}


/* Erodes mask by delegation to paint-funcs.c */
void Siox::erodeMask (Q_INT32 x, Q_INT32 y, Q_UINT32 width, Q_UINT32 height){
    m_selectionManager->erode();
}

/* Dilates mask by delegation to paint-funcs.c */
void Siox::dilateMask (Q_INT32 x, Q_INT32 y, Q_UINT32 width, Q_UINT32 height){
    m_selectionManager->dilate();
}

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

void Siox::findMaxBlob (Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height, const Q_INT32 sizeFactor)
{
    std::stack <Blob*> s;
    Q_INT32 maxsize = 0;
     
    thresholdMask (x, y, width, height);
    
    KisRectIteratorPixel maskIt = m_mask->createRectIterator(x, y, width, height, true);
    while( ! maskIt.isDone() ) {
        Q_UINT8 val = maskIt.rawData()[0]; //depth of mask is one byte
        if (val && (val != FIND_BLOB_VISITED)) {
            Blob *b = new Blob;
            b->seedx    = maskIt.x();
            b->seedy    = maskIt.y();
            b->size     = 0;
            b->mustkeep = FALSE;

            depthFirstSearch (x, y, x + width, y + height, b, FIND_BLOB_VISITED);

            s.push(b);

            if (b->size > maxsize) maxsize = b->size; 
        }
        ++maskIt;
    }
    while(!s.empty()) {
        Blob *b = s.top();
        s.pop();
        
        depthFirstSearch (x, y, x + width, y + height, b, (b->mustkeep || (b->size * sizeFactor >= maxsize)) ? FIND_BLOB_FINAL : 0);
        delete b;
    }
}

/* Returns squared clustersize */
float Siox::getClustersize (const float *limits)
{
  return ((limits[0] - (-limits[0])) * (limits[0] - (-limits[0])) +
          (limits[1] - (-limits[1])) * (limits[1] - (-limits[1])) +
          (limits[2] - (-limits[2])) * (limits[2] - (-limits[2])));
}

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

#define MULTIBLOB_DEFAULT_SIZEFACTOR 4
#define MULTIBLOB_ONE_BLOB_ONLY      0

void Siox::foregroundExtract(SioxRefinementType refinement, Q_INT32 x1, Q_INT32 y1, Q_INT32 x2, Q_INT32 y2, Q_INT32 smoothness, const double sensitivity[3], bool multiblob)
{
    std::vector<CieLab> surefg;
    std::vector<CieLab> surebg;

    limits[0] = sensitivity[0];
    limits[1] = sensitivity[1];
    limits[2] = sensitivity[2];

    float clustersize = getClustersize (limits);

    std::map<int, Classresult>::iterator cacheIt;
    if (refinement & SIOX_REFINEMENT_ADD_FOREGROUND) {
        for(cacheIt = m_cache.begin(); cacheIt != m_cache.end(); cacheIt++) {
            if(cacheRemoveBg(cacheIt->second)) m_cache.erase(cacheIt);
        }
    }
    if (refinement & SIOX_REFINEMENT_ADD_BACKGROUND) {
        for(cacheIt = m_cache.begin(); cacheIt != m_cache.end(); cacheIt++) {
            if(cacheRemoveFg(cacheIt->second)) m_cache.erase(cacheIt);
        }
    }
    if (refinement & SIOX_REFINEMENT_CHANGE_SENSITIVITY) {
        refinement = SIOX_REFINEMENT_RECALCULATE;
    } else {
        if (!m_bgSig.empty()) refinement = static_cast<SioxRefinementType>(refinement | SIOX_REFINEMENT_ADD_BACKGROUND);
        if (!m_fgSig.empty()) refinement = static_cast<SioxRefinementType>(refinement | SIOX_REFINEMENT_ADD_FOREGROUND);
    }

    if (refinement & (SIOX_REFINEMENT_ADD_FOREGROUND | SIOX_REFINEMENT_ADD_BACKGROUND)) {
        /* count given foreground and background pixels */
        //XXXX: this could probably be optimized by counting the pixels and resize the vectors to the number of pixels. Try this if it is too slow
        /* create inputs for color signatures */
        if (! (refinement & SIOX_REFINEMENT_ADD_FOREGROUND)) {
            KisRectIteratorPixel maskIt = m_mask->createRectIterator(m_x, m_y, m_width, m_height, false);
            KisRectIteratorPixel it = m_dev->createRectIterator(m_x, m_y, m_width, m_height, false);
            while( ! maskIt.isDone() ) {
                if(maskIt.rawData()[0] < SIOX_LOW){
                    surebg.push_back(CieLab(getRGB(it.rawData()[0],it.rawData()[1],it.rawData()[2],it.rawData()[3])));
                }
                ++maskIt;
                ++it;
            }
        }
        else if (! (refinement & SIOX_REFINEMENT_ADD_BACKGROUND)) {
            KisRectIteratorPixel maskIt = m_mask->createRectIterator(m_x, m_y, m_width, m_height, false);
            KisRectIteratorPixel it = m_dev->createRectIterator(m_x, m_y, m_width, m_height, false);
            while( ! maskIt.isDone() ) {
                if(maskIt.rawData()[0] > SIOX_HIGH){
                    surefg.push_back(CieLab(getRGB(it.rawData()[0],it.rawData()[1],it.rawData()[2],it.rawData()[3])));
                }
                ++maskIt;
                ++it;
            }
        }
        else { /* both changed */
            KisRectIteratorPixel maskIt = m_mask->createRectIterator(m_x, m_y, m_width, m_height, false);
            KisRectIteratorPixel it = m_dev->createRectIterator(m_x, m_y, m_width, m_height, false);
            while( ! maskIt.isDone() ) {
                if(maskIt.rawData()[0] < SIOX_LOW){
                    surebg.push_back(CieLab(getRGB(it.rawData()[0],it.rawData()[1],it.rawData()[2],it.rawData()[3])));
                }
                else if(maskIt.rawData()[0] > SIOX_HIGH){
                    surefg.push_back(CieLab(getRGB(it.rawData()[0],it.rawData()[1],it.rawData()[2],it.rawData()[3])));
                }
                ++maskIt;
                ++it;
            }
        }
        if (refinement & SIOX_REFINEMENT_ADD_BACKGROUND) {
            /* Create color signature for the background */
            //XXXX: replace SIOX_COLOR_DIMS by the correct dimension (for example, one for greyscale)
            colorSignature(surebg, m_bgSig, SIOX_COLOR_DIMS);
            surebg.clear();
            /*if (state->bgsiglen < 1)
            {
              g_free (surefg);
              return;
            }*/
        }
        if (refinement & SIOX_REFINEMENT_ADD_FOREGROUND) {
            /* Create color signature for the foreground */
            //XXXX: replace SIOX_COLOR_DIMS by the correct dimension (for example, one for greyscale)
            colorSignature(surefg, m_fgSig, SIOX_COLOR_DIMS);
            surefg.clear();
        }

    }

    /* Reduce the working area to the region of interest */
    int x      = x1;
    int y      = y1;
    int width  = x2 - x1;
    int height = y2 - y1;

    // Classify - the cached way....Better: Tree traversation? 
    int total = width * height;
    KisRectIteratorPixel maskIt = m_mask->createRectIterator(x, y, width, height, false);
    KisRectIteratorPixel it = m_dev->createRectIterator(x, y, width, height, true);
    while( ! maskIt.isDone() ) {
        float       minbg, minfg, d;
        Classresult *cr;
        if(maskIt.rawData()[0] < SIOX_LOW || maskIt.rawData()[0] > SIOX_HIGH) {
            ++maskIt;
            ++it;
            continue;
        }
        int key = createKey(it.rawData(),SIOX_COLOR_DIMS);
        cr = &m_cache.find(key)->second;
        if (cr)
        {
            maskIt.rawData()[0] = (cr->bgdist >= cr->fgdist) ? 254 : 0;
            ++maskIt;
            ++it;
            continue;
        }
        cr = new Classresult; //XXXX: cr = g_new0 (classresult, 1); initialize cr with zeros?
        CieLab labpixel(getRGB(it.rawData()[0],it.rawData()[1],it.rawData()[2],it.rawData()[3]));
        minbg = sqrEuclidianDist(labpixel, m_bgSig[0]);
        std::vector<CieLab>::iterator bgSigIt;
        bgSigIt = m_bgSig.begin();
        bgSigIt++; //skip the first element, we already processed it
        while(bgSigIt != m_bgSig.end())
        {
            d = sqrEuclidianDist(labpixel, *bgSigIt);
            if (d < minbg) minbg = d;
            bgSigIt++;
        }
        cr->bgdist = minbg;
     
        if (m_fgSig.empty()) {
            if (minbg < clustersize) minfg = minbg + clustersize;
            else minfg = 0.00001; // This is a guess -
                                 //   now we actually require a foreground
                                 //   signature, !=0 to avoid div by zero
        }
        else
        {
            minfg = sqrEuclidianDist (labpixel, m_fgSig[0]);

            std::vector<CieLab>::iterator fgSigIt;
            fgSigIt = m_fgSig.begin();
            fgSigIt++; //skip the first element, we already processed it
            while(bgSigIt != m_bgSig.end())
            {
                d = sqrEuclidianDist(labpixel, *fgSigIt);
                if (d < minfg) minfg = d;
                bgSigIt++;
            }
        }
        cr->fgdist = minfg;
        m_cache.insert(std::make_pair(key,*cr));

        maskIt.rawData()[0] = (cr->bgdist >= cr->fgdist) ? 254 : 0;
        ++maskIt;
        ++it;
    }
    // smooth a bit for error killing
    smoothMask (x, y, width, height);
    
    // erode, to make sure only "strongly connected components" keep being connected
    erodeMask (x, y, width, height);


    // search the biggest connected component
    findMaxBlob (x, y, width, height, multiblob ? MULTIBLOB_DEFAULT_SIZEFACTOR : MULTIBLOB_ONE_BLOB_ONLY);

    // smooth again - as user specified
    for (int n = 0; n < smoothness; n++) smoothMask (x, y, width, height);

    // search the biggest connected component again to kill jitter 
    findMaxBlob (x, y, width, height, multiblob ? MULTIBLOB_DEFAULT_SIZEFACTOR : MULTIBLOB_ONE_BLOB_ONLY);

    // dilate, to fill up boundary pixels killed by erode 
    dilateMask (x, y, width, height);

}

bool Siox::cacheRemoveBg (Classresult& value)
{
  Classresult cr = value;

  return (cr.bgdist < cr.fgdist);
}

bool Siox::cacheRemoveFg (Classresult& value)
{
  Classresult cr = value;

  return (cr.bgdist >= cr.fgdist);
}

unsigned long Siox::getRGB(int a, int r, int g, int b)
{
    if (a<0)  a=0;
    else if (a>255) a=255;

    if (r<0) r=0;
    else if (r>255) r=255;

    if (g<0) g=0;
    else if (g>255) g=255;

    if (b<0) b=0;
    else if (b>255) b=255;

    return (a<<24)|(r<<16)|(g<<8)|b;
}

#define RED_PIX          0
#define GREEN_PIX        1
#define BLUE_PIX         2
/* Creates a key for the hashtable from a given pixel color value */
int Siox::createKey (const unsigned char *src, int bpp)
{
    switch (bpp)
    {
    case 3:                     /* RGB  */
    case 4:                     /* RGBA */
      return (src[RED_PIX] << 16 | src[GREEN_PIX] << 8 | src[BLUE_PIX]);
    default:
      return 0;
    }
}

/*  FIXME: turn this into an enum  */
#define SIOX_DRB_ADD                0
#define SIOX_DRB_SUBTRACT           1


void Siox::drb(int x, int y, int brushRadius, int brushMode, float threshold)
{
    KisRectIteratorPixel maskIt = m_mask->createRectIterator(x - brushRadius, y - brushRadius, brushRadius * 2, brushRadius * 2, false);
    KisRectIteratorPixel it = m_dev->createRectIterator(x - brushRadius, y - brushRadius, brushRadius * 2, brushRadius * 2, true);
    while( ! maskIt.isDone() ) {
        float mindistbg;
        float mindistfg;
        float alpha;

        int key = createKey(it.rawData(),SIOX_COLOR_DIMS);
        Classresult* cr = &m_cache.find(key)->second;
        if (! cr) continue; // Unknown color - can only be sure background or sure forground 
        
        mindistbg = (float) sqrt(cr->bgdist);
        mindistfg = (float) sqrt(cr->fgdist);
        
        if (brushMode == SIOX_DRB_ADD)
        {
            if (maskIt.rawData()[0] > SIOX_HIGH)
            continue;

            if (mindistfg == 0.0)
            alpha = 1.0; // avoid div by zero
            else
            alpha = MIN (mindistbg / mindistfg, 1.0);
        }
        else //if (brush_mode == SIOX_DRB_SUBTRACT)
        {
            if (maskIt.rawData()[0] < SIOX_HIGH)
            continue;

            if (mindistbg == 0.0)
            alpha = 0.0; // avoid div by zero 
            else
            alpha = 1.0 - MIN (mindistfg / mindistbg, 1.0);
        }
        if (alpha < threshold)
        {
            /* background with a certain confidence
            * to be decided by user.
            */
            maskIt.rawData()[0] = 0;
        }
        else
        {
            maskIt.rawData()[0] = (int) (255.999 * alpha);
        }

    }
}


