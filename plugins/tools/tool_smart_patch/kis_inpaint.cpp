/*
 *  Copyright (c) 2017 Eugene Ingerman
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/**
 * Inpaint using the PatchMatch Algorithm
 *
 * | PatchMatch : A Randomized Correspondence Algorithm for Structural Image Editing
 * | by Connelly Barnes and Eli Shechtman and Adam Finkelstein and Dan B Goldman
 * | ACM Transactions on Graphics (Proc. SIGGRAPH), vol.28, aug-2009
 *
 * Original author Xavier Philippeau
 * Code adopted from: David Chatting https://github.com/davidchatting/PatchMatch
 */

#include <boost/multi_array.hpp>
#include <random>
#include <iostream>
#include <functional>


#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_selection.h"

#include "kis_debug.h"
#include "kis_paint_device_debug_utils.h"
//#include "kis_random_accessor_ng.h"

#include <QList>
#include <kis_transform_worker.h>
#include <kis_filter_strategy.h>
#include "KoColor.h"
#include "KoColorSpace.h"
#include "KoChannelInfo.h"
#include "KoMixColorsOp.h"
#include "KoColorModelStandardIds.h"
#include "KoColorSpaceRegistry.h"
#include "KoColorSpaceTraits.h"

const int MAX_DIST = 65535;
const quint8 MASK_SET = 255;
const quint8 MASK_CLEAR = 0;

class MaskedImage; //forward decl for the forward decl below
template <typename T> float distance_impl(const MaskedImage& my, int x, int y, const MaskedImage& other, int xo, int yo);


class ImageView
{

protected:
    quint8* m_data;
    int m_imageWidth;
    int m_imageHeight;
    int m_pixelSize;

public:
    void Init(quint8* _data, int _imageWidth, int _imageHeight, int _pixelSize)
    {
        m_data = _data;
        m_imageWidth = _imageWidth;
        m_imageHeight = _imageHeight;
        m_pixelSize = _pixelSize;
    }

    ImageView() : m_data(nullptr)

    {
        m_imageHeight =  m_imageWidth = m_pixelSize = 0;
    }


    ImageView(quint8* _data, int _imageWidth, int _imageHeight, int _pixelSize)
    {
        Init(_data, _imageWidth, _imageHeight, _pixelSize);
    }

    quint8* operator()(int x, int y) const
    {
        Q_ASSERT(m_data);
        Q_ASSERT((x >= 0) && (x < m_imageWidth) && (y >= 0) && (y < m_imageHeight));
        return (m_data + x * m_pixelSize + y * m_imageWidth * m_pixelSize);
    }

    ImageView& operator=(const ImageView& other)
    {
        if (this != &other) {
            if (other.num_bytes() != num_bytes()) {
                delete[] m_data;
                m_data = nullptr; //to preserve invariance if next line throws exception
                m_data = new quint8[other.num_bytes()];

            }
            std::copy(other.data(), other.data() + other.num_bytes(), m_data);
            m_imageHeight = other.m_imageHeight;
            m_imageWidth = other.m_imageWidth;
            m_pixelSize = other.m_pixelSize;
        }
        return *this;
    }

    //move assignment operator
    ImageView& operator=(ImageView&& other) noexcept
    {
        if (this != &other) {
            delete[] m_data;
            m_data = nullptr;
            Init(other.data(), other.m_imageWidth, other.m_imageHeight, other.m_pixelSize);
            other.m_data = nullptr;
        }
        return *this;
    }

    virtual ~ImageView() {} //this class doesn't own m_data, so it ain't going to delete it either.

    quint8* data(void) const
    {
        return m_data;
    }

    inline int num_elements(void) const
    {
        return m_imageHeight * m_imageWidth;
    }

    inline int num_bytes(void) const
    {
        return m_imageHeight * m_imageWidth * m_pixelSize;
    }

    inline int pixel_size(void) const
    {
        return m_pixelSize;
    }

    void saveToDevice(KisPaintDeviceSP outDev, QRect rect)
    {
        Q_ASSERT(outDev->colorSpace()->pixelSize() == (quint32) m_pixelSize);
        outDev->writeBytes(m_data, rect);
    }

    void DebugDump(const QString& fnamePrefix)
    {
        QRect imSize(QPoint(0, 0), QSize(m_imageWidth, m_imageHeight));
        const KoColorSpace* cs = (m_pixelSize == 1) ?
                                 KoColorSpaceRegistry::instance()->alpha8() : (m_pixelSize == 3) ? KoColorSpaceRegistry::instance()->colorSpace("RGB", "U8", "") :
                                 KoColorSpaceRegistry::instance()->colorSpace("RGBA", "U8", "");
        KisPaintDeviceSP dbout = new KisPaintDevice(cs);
        saveToDevice(dbout, imSize);
        KIS_DUMP_DEVICE_2(dbout, imSize, fnamePrefix, "./");
    }
};

class ImageData : public ImageView
{

public:
    ImageData() : ImageView() {}

    void Init(int _imageWidth, int _imageHeight, int _pixelSize)
    {
        m_data = new quint8[ _imageWidth * _imageHeight * _pixelSize ];
        ImageView::Init(m_data, _imageWidth, _imageHeight, _pixelSize);
    }

    ImageData(int _imageWidth, int _imageHeight, int _pixelSize) : ImageView()
    {
        Init(_imageWidth, _imageHeight, _pixelSize);
    }

    void Init(KisPaintDeviceSP imageDev, const QRect& imageSize)
    {
        const KoColorSpace* cs = imageDev->colorSpace();
        m_pixelSize = cs->pixelSize();

        m_data = new quint8[ imageSize.width()*imageSize.height()*cs->pixelSize() ];
        imageDev->readBytes(m_data, imageSize.x(), imageSize.y(), imageSize.width(), imageSize.height());
        ImageView::Init(m_data, imageSize.width(), imageSize.height(), m_pixelSize);
    }

    ImageData(KisPaintDeviceSP imageDev, const QRect& imageSize) : ImageView()
    {
        Init(imageDev, imageSize);
    }

    ~ImageData() override
    {
        delete[] m_data; //ImageData owns m_data, so it has to delete it
    }

};



class MaskedImage : public KisShared
{
private:

    template <typename T> friend float distance_impl(const MaskedImage& my, int x, int y, const MaskedImage& other, int xo, int yo);

    QRect imageSize;
    int nChannels;

    const KoColorSpace* cs;
    const KoColorSpace* csMask;

    ImageData maskData;
    ImageData imageData;


    void cacheImage(KisPaintDeviceSP imageDev, QRect rect)
    {
        cs = imageDev->colorSpace();
        nChannels = cs->channelCount();
        imageData.Init(imageDev, rect);
        imageSize = rect;
    }


    void cacheMask(KisPaintDeviceSP maskDev, QRect rect)
    {
        Q_ASSERT(maskDev->colorSpace()->pixelSize() == 1);
        csMask = maskDev->colorSpace();
        maskData.Init(maskDev, rect);

        //hard threshold for the initial mask
        //may be optional. needs testing
        std::for_each(maskData.data(), maskData.data() + maskData.num_bytes(), [](quint8 & v) {
            v = (v > MASK_CLEAR) ? MASK_SET : MASK_CLEAR;
        });
    }

    MaskedImage() {}

public:
    std::function< float(const MaskedImage&, int, int, const MaskedImage& , int , int ) > distance;

    void toPaintDevice(KisPaintDeviceSP imageDev, QRect rect, KisSelectionSP selection)
    {
        if (!selection) {
            imageData.saveToDevice(imageDev, rect);
        } else {
            KisPaintDeviceSP dev = new KisPaintDevice(imageDev->colorSpace());
            dev->setDefaultBounds(imageDev->defaultBounds());

            imageData.saveToDevice(dev, rect);

            KisPainter::copyAreaOptimized(rect.topLeft(), dev, imageDev, rect, selection);
        }
    }

    void DebugDump(const QString& name)
    {
        imageData.DebugDump(name + "_img");
        maskData.DebugDump(name + "_mask");
    }

    void clearMask(void)
    {
        std::fill(maskData.data(), maskData.data() + maskData.num_bytes(), MASK_CLEAR);
    }

    void initialize(KisPaintDeviceSP _imageDev, KisPaintDeviceSP _maskDev, QRect _maskRect)
    {
        cacheImage(_imageDev, _maskRect);
        cacheMask(_maskDev, _maskRect);

        //distance function is the only that needs to know the type
        //For performance reasons we can't use functions provided by color space
        KoID colorDepthId =  _imageDev->colorSpace()->colorDepthId();

        //Use RGB traits to assign actual pixel data types.
        distance = &distance_impl<KoRgbU8Traits::channels_type>;

        if( colorDepthId == Integer16BitsColorDepthID )
            distance = &distance_impl<KoRgbU16Traits::channels_type>;
#ifdef HAVE_OPENEXR
        if( colorDepthId == Float16BitsColorDepthID )
            distance = &distance_impl<KoRgbF16Traits::channels_type>;
#endif
        if( colorDepthId == Float32BitsColorDepthID )
            distance = &distance_impl<KoRgbF32Traits::channels_type>;

        if( colorDepthId == Float64BitsColorDepthID )
            distance = &distance_impl<KoRgbF64Traits::channels_type>;
    }

    MaskedImage(KisPaintDeviceSP _imageDev, KisPaintDeviceSP _maskDev, QRect _maskRect)
    {
        initialize(_imageDev, _maskDev, _maskRect);
    }

    void downsample2x(void)
    {
        int H = imageSize.height();
        int W = imageSize.width();
        int newW = W / 2, newH = H / 2;

        KisPaintDeviceSP imageDev = new KisPaintDevice(cs);
        KisPaintDeviceSP maskDev = new KisPaintDevice(csMask);
        imageDev->writeBytes(imageData.data(), 0, 0, W, H);
        maskDev->writeBytes(maskData.data(), 0, 0, W, H);

        ImageData newImage(newW, newH, cs->pixelSize());
        ImageData newMask(newW, newH, 1);

        KoDummyUpdater updater;
        KisTransformWorker worker(imageDev, 1. / 2., 1. / 2., 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                                  &updater, KisFilterStrategyRegistry::instance()->value("Bicubic"));
        worker.run();

        KisTransformWorker workerMask(maskDev, 1. / 2., 1. / 2., 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                                      &updater, KisFilterStrategyRegistry::instance()->value("Bicubic"));
        workerMask.run();

        imageDev->readBytes(newImage.data(), 0, 0, newW, newH);
        maskDev->readBytes(newMask.data(), 0, 0, newW, newH);
        imageData = std::move(newImage);
        maskData = std::move(newMask);

        for (int i = 0; i < imageData.num_elements(); ++i) {
            quint8* maskPix = maskData.data() + i * maskData.pixel_size();
            if (*maskPix == MASK_SET) {
                for (int k = 0; k < imageData.pixel_size(); k++)
                    *(imageData.data() + i * imageData.pixel_size() + k) = 0;
            } else {
                *maskPix = MASK_CLEAR;
            }
        }
        imageSize = QRect(0, 0, newW, newH);
    }

    void upscale(int newW, int newH)
    {
        int H = imageSize.height();
        int W = imageSize.width();

        ImageData newImage(newW, newH, cs->pixelSize());
        ImageData newMask(newW, newH, 1);

        QVector<float> colors(nChannels, 0.f);
        QVector<float> v(nChannels, 0.f);

        for (int y = 0; y < newH; ++y) {
            for (int x = 0; x < newW; ++x) {

                // original pixel
                int xs = (x * W) / newW;
                int ys = (y * H) / newH;

                // copy to new image
                if (!isMasked(xs, ys)) {
                    std::copy(imageData(xs, ys), imageData(xs, ys) + imageData.pixel_size(), newImage(x, y));
                    *newMask(x, y) = MASK_CLEAR;
                } else {
                    std::fill(newImage(x, y), newImage(x, y) + newImage.pixel_size(), 0);
                    *newMask(x, y) = MASK_SET;
                }
            }
        }

        imageData = std::move(newImage);
        maskData = std::move(newMask);
        imageSize = QRect(0, 0, newW, newH);
    }

    QRect size()
    {
        return imageSize;
    }

    KisSharedPtr<MaskedImage> copy(void)
    {
        KisSharedPtr<MaskedImage> clone = new MaskedImage();
        clone->imageSize = this->imageSize;
        clone->nChannels = this->nChannels;
        clone->maskData = this->maskData;
        clone->imageData = this->imageData;
        clone->cs = this->cs;
        clone->csMask = this->csMask;
        clone->distance = this->distance;
        return clone;
    }

    int countMasked(void)
    {
        int count = std::count_if(maskData.data(), maskData.data() + maskData.num_elements(), [](quint8 v) {
            return v > MASK_CLEAR;
        });
        return count;
    }

    inline bool isMasked(int x, int y)
    {
        return (*maskData(x, y) > MASK_CLEAR);
    }

    //returns true if the patch contains a masked pixel
    bool containsMasked(int x, int y, int S)
    {
        for (int dy = -S; dy <= S; ++dy) {
            int ys = y + dy;
            if (ys < 0 || ys >= imageSize.height())
                continue;

            for (int dx = -S; dx <= S; ++dx) {
                int xs = x + dx;
                if (xs < 0 || xs >= imageSize.width())
                    continue;
                if (isMasked(xs, ys))
                    return true;
            }
        }
        return false;
    }

    inline quint8 getImagePixelU8(int x, int y, int chan) const
    {
        return cs->scaleToU8(imageData(x, y), chan);
    }

    inline QVector<float> getImagePixels(int x, int y) const
    {
        QVector<float> v(cs->channelCount());
        cs->normalisedChannelsValue(imageData(x, y), v);
        return v;
    }

    inline quint8* getImagePixel(int x, int y)
    {
        return imageData(x, y);
    }

    inline void setImagePixels(int x, int y, QVector<float>& value)
    {
        cs->fromNormalisedChannelsValue(imageData(x, y), value);
    }

    inline void mixColors(std::vector< quint8* > pixels, std::vector< float > w, float wsum,  quint8* dst)
    {
        const KoMixColorsOp* mixOp = cs->mixColorsOp();

        size_t n = w.size();
        assert(pixels.size() == n);
        std::vector< qint16 > weights;
        weights.clear();

        float dif = 0;

        float scale = 255 / (wsum + 0.001);

        for (auto& v : w) {
            //compensated summation to increase accuracy
            float v1 = v * scale + dif;
            float v2 = std::round(v1);
            dif = v1 - v2;
            weights.push_back(v2);
        }

        mixOp->mixColors(pixels.data(), weights.data(), n, dst);
    }

    inline void setMask(int x, int y, quint8 v)
    {
        *(maskData(x, y)) = v;
    }

    inline int channelCount(void) const
    {
        return cs->channelCount();
    }
};


//Generic version of the distance function. produces distance between colors in the range [0, MAX_DIST]. This
//is a fast distance computation. More accurate, but very slow implementation is to use color space operations.
template <typename T> float distance_impl(const MaskedImage& my, int x, int y, const MaskedImage& other, int xo, int yo)
{
    float dsq = 0;
    quint32 nchannels = my.channelCount();
    T *v1 = reinterpret_cast<T*>(my.imageData(x, y));
    T *v2 = reinterpret_cast<T*>(other.imageData(xo, yo));

    for (quint32 chan = 0; chan < nchannels; chan++) {
        //It's very important not to lose precision in the next line
        float v = ((float)(*(v1 + chan)) - (float)(*(v2 + chan)));
        dsq += v * v;
    }
    return dsq / (KoColorSpaceMathsTraits<float>::unitValue * KoColorSpaceMathsTraits<float>::unitValue / MAX_DIST );
}


typedef KisSharedPtr<MaskedImage> MaskedImageSP;

struct NNPixel {
    int x;
    int y;
    int distance;
};
typedef boost::multi_array<NNPixel, 2> NNArray_type;

struct Vote_elem {
    QVector<float> channel_values;
    float w;
};
typedef boost::multi_array<Vote_elem, 2> Vote_type;



class NearestNeighborField : public KisShared
{

private:
    template< typename T> T randomInt(T range)
    {
        return rand() % range;
    }

    //compute initial value of the distance term
    void initialize(void)
    {
        for (int y = 0; y < imSize.height(); y++) {
            for (int x = 0; x < imSize.width(); x++) {
                field[x][y].distance = distance(x, y, field[x][y].x, field[x][y].y);

                //if the distance is "infinity", try to find a better link
                int iter = 0;
                const int maxretry = 20;
                while (field[x][y].distance == MAX_DIST && iter < maxretry) {
                    field[x][y].x = randomInt(imSize.width() + 1);
                    field[x][y].y = randomInt(imSize.height() + 1);
                    field[x][y].distance = distance(x, y, field[x][y].x, field[x][y].y);
                    iter++;
                }
            }
        }
    }

    void init_similarity_curve(void)
    {
        float s_zero = 0.999;
        float t_halfmax = 0.10;

        float x  = (s_zero - 0.5) * 2;
        float invtanh = 0.5 * std::log((1. + x) / (1. - x));
        float coef = invtanh / t_halfmax;

        similarity.resize(MAX_DIST + 1);
        for (int i = 0; i < (int)similarity.size(); i++) {
            float t = (float)i / similarity.size();
            similarity[i] = 0.5 - 0.5 * std::tanh(coef * (t - t_halfmax));
        }
    }


private:
    int patchSize; //patch size
public:
    MaskedImageSP input;
    MaskedImageSP output;
    QRect imSize;
    NNArray_type field;
    std::vector<float> similarity;
    quint32 nColors;
    QList<KoChannelInfo *> channels;

public:
    NearestNeighborField(const MaskedImageSP _input, MaskedImageSP _output, int _patchsize) : patchSize(_patchsize), input(_input), output(_output)
    {
        imSize = input->size();
        field.resize(boost::extents[imSize.width()][imSize.height()]);
        init_similarity_curve();

        nColors = input->channelCount(); //only color count, doesn't include alpha channels
    }

    void randomize(void)
    {
        for (int y = 0; y < imSize.height(); y++) {
            for (int x = 0; x < imSize.width(); x++) {
                field[x][y].x = randomInt(imSize.width() + 1);
                field[x][y].y = randomInt(imSize.height() + 1);
                field[x][y].distance = MAX_DIST;
            }
        }
        initialize();
    }

    //initialize field from an existing (possibly smaller) nearest neighbor field
    void initialize(const NearestNeighborField& nnf)
    {
        float xscale = qreal(imSize.width()) / nnf.imSize.width();
        float yscale = qreal(imSize.height()) / nnf.imSize.height();

        for (int y = 0; y < imSize.height(); y++) {
            for (int x = 0; x < imSize.width(); x++) {
                int xlow = std::min((int)(x / xscale), nnf.imSize.width() - 1);
                int ylow = std::min((int)(y / yscale), nnf.imSize.height() - 1);

                field[x][y].x = nnf.field[xlow][ylow].x * xscale;
                field[x][y].y = nnf.field[xlow][ylow].y * yscale;
                field[x][y].distance = MAX_DIST;
            }
        }
        initialize();
    }

    //multi-pass NN-field minimization (see "PatchMatch" paper referenced above - page 4)
    void minimize(int pass)
    {
        int min_x = 0;
        int min_y = 0;
        int max_x = imSize.width() - 1;
        int max_y = imSize.height() - 1;

        for (int i = 0; i < pass; i++) {
            //scanline order
            for (int y = min_y; y < max_y; y++)
                for (int x = min_x; x <= max_x; x++)
                    if (field[x][y].distance > 0)
                        minimizeLink(x, y, 1);

            //reverse scanline order
            for (int y = max_y; y >= min_y; y--)
                for (int x = max_x; x >= min_x; x--)
                    if (field[x][y].distance > 0)
                        minimizeLink(x, y, -1);
        }
    }

    void minimizeLink(int x, int y, int dir)
    {
        int xp, yp, dp;

        //Propagation Left/Right
        if (x - dir > 0 && x - dir < imSize.width()) {
            xp = field[x - dir][y].x + dir;
            yp = field[x - dir][y].y;
            dp = distance(x, y, xp, yp);
            if (dp < field[x][y].distance) {
                field[x][y].x = xp;
                field[x][y].y = yp;
                field[x][y].distance = dp;
            }
        }

        //Propagation Up/Down
        if (y - dir > 0 && y - dir < imSize.height()) {
            xp = field[x][y - dir].x;
            yp = field[x][y - dir].y + dir;
            dp = distance(x, y, xp, yp);
            if (dp < field[x][y].distance) {
                field[x][y].x = xp;
                field[x][y].y = yp;
                field[x][y].distance = dp;
            }
        }

        //Random search
        int wi = std::max(output->size().width(), output->size().height());
        int xpi = field[x][y].x;
        int ypi = field[x][y].y;
        while (wi > 0) {
            xp = xpi + randomInt(2 * wi) - wi;
            yp = ypi + randomInt(2 * wi) - wi;
            xp = std::max(0, std::min(output->size().width() - 1, xp));
            yp = std::max(0, std::min(output->size().height() - 1, yp));

            dp = distance(x, y, xp, yp);
            if (dp < field[x][y].distance) {
                field[x][y].x = xp;
                field[x][y].y = yp;
                field[x][y].distance = dp;
            }
            wi /= 2;
        }
    }

    //compute distance between two patches
    int distance(int x, int y, int xp, int yp)
    {
        float distance = 0;
        float wsum = 0;
        float ssdmax = nColors * 255 * 255;

        //for each pixel in the source patch
        for (int dy = -patchSize; dy <= patchSize; dy++) {
            for (int dx = -patchSize; dx <= patchSize; dx++) {
                wsum += ssdmax;
                int xks = x + dx;
                int yks = y + dy;

                if (xks < 0 || xks >= input->size().width()) {
                    distance += ssdmax;
                    continue;
                }

                if (yks < 0 || yks >= input->size().height()) {
                    distance += ssdmax;
                    continue;
                }

                //cannot use masked pixels as a valid source of information
                if (input->isMasked(xks, yks)) {
                    distance += ssdmax;
                    continue;
                }

                //corresponding pixel in target patch
                int xkt = xp + dx;
                int ykt = yp + dy;
                if (xkt < 0 || xkt >= output->size().width()) {
                    distance += ssdmax;
                    continue;
                }
                if (ykt < 0 || ykt >= output->size().height()) {
                    distance += ssdmax;
                    continue;
                }

                //cannot use masked pixels as a valid source of information
                if (output->isMasked(xkt, ykt)) {
                    distance += ssdmax;
                    continue;
                }

                //SSD distance between pixels
                float ssd = input->distance(*input, xks, yks, *output, xkt, ykt);
                distance += ssd;

            }
        }
        return (int)(MAX_DIST * (distance / wsum));
    }

    static MaskedImageSP ExpectationMaximization(KisSharedPtr<NearestNeighborField> TargetToSource, int level, int radius, QList<MaskedImageSP>& pyramid);

    static void ExpectationStep(KisSharedPtr<NearestNeighborField> nnf, MaskedImageSP source, MaskedImageSP target, bool upscale);

    void EM_Step(MaskedImageSP source, MaskedImageSP target, int R, bool upscaled);
};
typedef KisSharedPtr<NearestNeighborField> NearestNeighborFieldSP;


class Inpaint
{
private:
    KisPaintDeviceSP devCache;
    MaskedImageSP initial;
    NearestNeighborFieldSP nnf_TargetToSource;
    NearestNeighborFieldSP nnf_SourceToTarget;
    int radius;
    QList<MaskedImageSP> pyramid;


public:
    Inpaint(KisPaintDeviceSP dev, KisPaintDeviceSP devMask, int _radius, QRect maskRect)
    : devCache(dev)
    , initial(new MaskedImage(dev, devMask, maskRect))
    , radius(_radius)
    {
    }
    MaskedImageSP patch(void);
    MaskedImageSP patch_simple(void);
};



MaskedImageSP Inpaint::patch()
{
    MaskedImageSP source = initial->copy();

    pyramid.append(initial);

    QRect size = source->size();

    //qDebug() << "countMasked: " <<  source->countMasked() << "\n";
    while ((size.width() > radius) && (size.height() > radius) && source->countMasked() > 0) {
        source->downsample2x();
        //source->DebugDump("Pyramid");
        //qDebug() << "countMasked1: " <<  source->countMasked() << "\n";
        pyramid.append(source->copy());
        size = source->size();
    }
    int maxlevel = pyramid.size();
    //qDebug() << "MaxLevel: " <<  maxlevel << "\n";

    // The initial target is the same as the smallest source.
    // We consider that this target contains no masked pixels
    MaskedImageSP target = source->copy();
    target->clearMask();

    //recursively building nearest neighbor field
    for (int level = maxlevel - 1; level > 0; level--) {
        source = pyramid.at(level);

        if (level == maxlevel - 1) {
            //random initial guess
            nnf_TargetToSource = new NearestNeighborField(target, source, radius);
            nnf_TargetToSource->randomize();
        } else {
            // then, we use the rebuilt (upscaled) target
            // and reuse the previous NNF as initial guess

            NearestNeighborFieldSP new_nnf_rev = new NearestNeighborField(target, source, radius);
            new_nnf_rev->initialize(*nnf_TargetToSource);
            nnf_TargetToSource = new_nnf_rev;
        }

        //Build an upscaled target by EM-like algorithm (see "PatchMatch" paper referenced above - page 6)
        target = NearestNeighborField::ExpectationMaximization(nnf_TargetToSource, level, radius, pyramid);
        //target->DebugDump( "target" );
    }
    return target;
}


//EM-Like algorithm (see "PatchMatch" - page 6)
//Returns a float sized target image
MaskedImageSP NearestNeighborField::ExpectationMaximization(NearestNeighborFieldSP nnf_TargetToSource, int level, int radius, QList<MaskedImageSP>& pyramid)
{
    int iterEM = std::min(2 * level, 4);
    int iterNNF = std::min(5, 1 + level);

    MaskedImageSP source = nnf_TargetToSource->output;
    MaskedImageSP target = nnf_TargetToSource->input;
    MaskedImageSP newtarget = nullptr;

    //EM loop
    for (int emloop = 1; emloop <= iterEM; emloop++) {
        //set the new target as current target
        if (!newtarget.isNull()) {
            nnf_TargetToSource->input = newtarget;
            target = newtarget;
            newtarget = nullptr;
        }

        for (int x = 0; x < target->size().width(); ++x) {
            for (int y = 0; y < target->size().height(); ++y) {
                if (!source->containsMasked(x, y, radius)) {
                    nnf_TargetToSource->field[x][y].x = x;
                    nnf_TargetToSource->field[x][y].y = y;
                    nnf_TargetToSource->field[x][y].distance = 0;
                }
            }
        }

        //minimize the NNF
        nnf_TargetToSource->minimize(iterNNF);

        //Now we rebuild the target using best patches from source
        MaskedImageSP newsource = nullptr;
        bool upscaled = false;

        // Instead of upsizing the final target, we build the last target from the next level source image
        // So the final target is less blurry (see "Space-Time Video Completion" - page 5)
        if (level >= 1 && (emloop == iterEM)) {
            newsource = pyramid.at(level - 1);
            QRect sz = newsource->size();
            newtarget = target->copy();
            newtarget->upscale(sz.width(), sz.height());
            upscaled = true;
        } else {
            newsource = pyramid.at(level);
            newtarget = target->copy();
            upscaled = false;
        }
        //EM Step

        //EM_Step(newsource, newtarget, radius, upscaled);
        ExpectationStep(nnf_TargetToSource, newsource, newtarget, upscaled);
    }

    return newtarget;
}


void NearestNeighborField::ExpectationStep(NearestNeighborFieldSP nnf, MaskedImageSP source, MaskedImageSP target, bool upscale)
{
    //int*** field = nnf->field;
    int R = nnf->patchSize;
    if (upscale)
        R *= 2;

    int H_nnf = nnf->input->size().height();
    int W_nnf = nnf->input->size().width();
    int H_target = target->size().height();
    int W_target = target->size().width();
    int H_source = source->size().height();
    int W_source = source->size().width();

    std::vector< quint8* > pixels;
    std::vector< float > weights;
    pixels.reserve(R * R);
    weights.reserve(R * R);
    for (int x = 0 ; x < W_target ; ++x) {
        for (int y = 0 ; y < H_target; ++y) {
            float wsum = 0;
            pixels.clear();
            weights.clear();


            if (!source->containsMasked(x, y, R + 4) /*&& upscale*/) {
                //speedup computation by copying parts that are not masked.
                pixels.push_back(source->getImagePixel(x, y));
                weights.push_back(1.f);
                target->mixColors(pixels, weights, 1.f, target->getImagePixel(x, y));
            } else {
                for (int dx = -R ; dx <= R; ++dx) {
                    for (int dy = -R ; dy <= R ; ++dy) {
                        // xpt,ypt = center pixel of the target patch
                        int xpt = x + dx;
                        int ypt = y + dy;

                        int xst, yst;
                        float w;

                        if (!upscale) {
                            if (xpt < 0 || xpt >= W_nnf || ypt < 0 || ypt >= H_nnf)
                                continue;

                            xst = nnf->field[xpt][ypt].x;
                            yst = nnf->field[xpt][ypt].y;
                            float dp = nnf->field[xpt][ypt].distance;
                            // similarity measure between the two patches
                            w = nnf->similarity[dp];

                        } else {
                            if (xpt < 0 || (xpt / 2) >= W_nnf || ypt < 0 || (ypt / 2) >= H_nnf)
                                continue;
                            xst = 2 * nnf->field[xpt / 2][ypt / 2].x + (xpt % 2);
                            yst = 2 * nnf->field[xpt / 2][ypt / 2].y + (ypt % 2);
                            float dp = nnf->field[xpt / 2][ypt / 2].distance;
                            // similarity measure between the two patches
                            w = nnf->similarity[dp];
                        }

                        int xs = xst - dx;
                        int ys = yst - dy;

                        if (xs < 0 || xs >= W_source || ys < 0 || ys >= H_source)
                            continue;

                        if (source->isMasked(xs, ys))
                            continue;

                        pixels.push_back(source->getImagePixel(xs, ys));
                        weights.push_back(w);
                        wsum += w;
                    }
                }

                if (wsum < 1)
                    continue;

                target->mixColors(pixels, weights, wsum, target->getImagePixel(x, y));
            }
        }
    }
}

QRect getMaskBoundingBox(KisPaintDeviceSP maskDev)
{
    QRect maskRect = maskDev->nonDefaultPixelArea();
    return maskRect;
}


QRect patchImage(const KisPaintDeviceSP imageDev, const KisPaintDeviceSP maskDev, int patchRadius, int accuracy, KisSelectionSP selection)
{
    QRect maskRect = getMaskBoundingBox(maskDev);
    QRect imageRect = imageDev->exactBounds();

    float scale = 1.0 + (accuracy / 25.0); //higher accuracy means we include more surrounding area around the patch. Minimum 2x padding.
    int dx = maskRect.width() * scale;
    int dy = maskRect.height() * scale;
    maskRect.adjust(-dx, -dy, dx, dy);
    maskRect = maskRect.intersected(imageRect);

    if (!maskRect.isEmpty()) {
        Inpaint inpaint(imageDev, maskDev, patchRadius, maskRect);
        MaskedImageSP output = inpaint.patch();
        output->toPaintDevice(imageDev, maskRect, selection);
    }

    return maskRect;
}

