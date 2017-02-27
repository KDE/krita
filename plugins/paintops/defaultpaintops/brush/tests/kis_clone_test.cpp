/*
 *  Copyright (c) 2016 Eugene Ingerman
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

#include "kis_paint_device.h"

#include "kis_clone_test.h"
#include "kis_debug.h"
#include "kis_paint_device_debug_utils.h"
#include "kis_random_accessor_ng.h"

#include <QTest>

#include <QList>
#include <kis_transform_worker.h>
#include <kis_filter_strategy.h>
#include "KoColor.h"
#include "KoColorSpace.h"
#include "KoChannelInfo.h"
#include "KoMixColorsOp.h"
#include "KoColorModelStandardIds.h"

#include <KisPart.h>
#include <kis_group_layer.h>
#include <qimage_based_test.h>
#include <stroke_testing_utils.h>
#include <brushengine/kis_paint_information.h>
#include <kis_canvas_resource_provider.h>
#include <brushengine/kis_paintop_preset.h>
#include <brushengine/kis_paintop_settings.h>

#if 1
#include "hdf5.h"
#include "hdf5_hl.h"
#endif

#define isOdd(x) ((x) & 0x01)

const int MAX_DIST = 65535;
const quint8 MASK_SET = 0;
const quint8 MASK_CLEAR = 255;

class DebugSaver {
    QString fname;
    hid_t file_id;
    int counter;

public:
    static DebugSaver* instance();

    DebugSaver() : counter(0) {};

    ~DebugSaver() {
        if( file_id != -1 )
            H5Fclose(file_id);
    }

    void openDebugFile( QString fname ){
        file_id = H5Fcreate(fname.toStdString().c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    }

    template< typename T> void debugDumpField( QString dset, const T& field )
    {
        if( file_id == -1 )
            return;

        QString dset_name = QString("/") + QString::number(counter);

        hsize_t dims[] = {field.shape()[0], field.shape()[1], 3}; //x,y,distance
        H5LTmake_dataset(file_id, dset_name.toStdString().c_str(), 3, dims, H5T_NATIVE_INT, (void*)field.data());
        H5LTset_attribute_string (file_id, dset_name.toStdString().c_str(), "name", dset.toStdString().c_str());

        counter++;
    }

    template< typename T> void debugDumpHistogram(QString dset, const T& histogram)
    {

        if( file_id == -1 )
            return;

        QString dset_name = QString("/") + QString::number(counter);
        H5LTmake_dataset(file_id, dset_name.toStdString().c_str(), histogram.num_dimensions(), (const hsize_t*)histogram.shape(), H5T_NATIVE_FLOAT, (void*)histogram.data());
        H5LTset_attribute_string (file_id, dset_name.toStdString().c_str(), "name", dset.toStdString().c_str());

        counter++;
    }
};

Q_GLOBAL_STATIC(DebugSaver, s_instance)

DebugSaver* DebugSaver::instance(){ return s_instance; }

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

    //move assignement operator
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

    virtual ~ImageView() {} //we don't own m_data, so we aren't going to delete it either.

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

    void saveToDevice(KisPaintDeviceSP outDev)
    {
        QRect imSize(QPoint(0, 0), QSize(m_imageWidth, m_imageHeight));
        Q_ASSERT(outDev->colorSpace()->pixelSize() == m_pixelSize);
        outDev->writeBytes(m_data, imSize);
    }

    void DebugDump(const QString& fnamePrefix)
    {
        QRect imSize(QPoint(0, 0), QSize(m_imageWidth, m_imageHeight));
        const KoColorSpace* cs = (m_pixelSize == 1) ?
                                 KoColorSpaceRegistry::instance()->alpha8() : (m_pixelSize == 3) ? KoColorSpaceRegistry::instance()->colorSpace("RGB", "U8", "") :
                                 KoColorSpaceRegistry::instance()->colorSpace("RGBA", "U8", "");
        KisPaintDeviceSP dbout = new KisPaintDevice(cs);
        saveToDevice(dbout);
        KIS_DUMP_DEVICE_2(dbout, imSize, fnamePrefix, "/home/eugening/Projects/img");
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

    virtual ~ImageData()
    {
        delete[] m_data; //ImageData owns m_data, so it has to delete it
    }

};

inline void alignRectBy2(qint32 &x, qint32 &y, qint32 &w, qint32 &h)
{
    x -= isOdd(x);
    y -= isOdd(y);
    w += isOdd(x);
    w += isOdd(w);
    h += isOdd(y);
    h += isOdd(h);
}


class MaskedImage : public KisShared
{
private:

    QRect imageSize;
    int nChannels;

    const KoColorSpace* cs;
    const KoColorSpace* csMask;

    ImageData maskData;
    ImageData imageData;


    void cacheImageSize(KisPaintDeviceSP imageDev)
    {
        imageSize = imageDev->exactBounds();
    }

    void cacheImage(KisPaintDeviceSP imageDev)
    {
        Q_ASSERT(!imageSize.isEmpty() && imageSize.isValid());
        cs = imageDev->colorSpace();
        nChannels = cs->channelCount();
        imageData.Init(imageDev, imageSize);
    }


    void cacheMask(KisPaintDeviceSP maskDev)
    {
        Q_ASSERT(!imageSize.isEmpty() && imageSize.isValid());
        Q_ASSERT(maskDev->colorSpace()->pixelSize() == 1);
        csMask = maskDev->colorSpace();
        maskData.Init(maskDev, imageSize);
    }

    MaskedImage() {}

public:

    void toPaintDevice(KisPaintDeviceSP imageDev)
    {
        imageData.saveToDevice(imageDev);
    }

    void DebugDump( const QString& name ){
        imageData.DebugDump( name );
    }

    void clearMask(void)
    {
        std::fill(maskData.data(), maskData.data() + maskData.num_bytes(), MASK_CLEAR);
    }

    void initialize(KisPaintDeviceSP _imageDev, KisPaintDeviceSP _maskDev)
    {
        cacheImageSize(_imageDev);
        cacheImage(_imageDev);
        cacheMask(_maskDev);
    }

//    void clone(const MaskeImageSP other)
//    {
//        return new MaskedImage(imageDev, maskDev);
//    }

    MaskedImage(KisPaintDeviceSP _imageDev, KisPaintDeviceSP _maskDev)
    {
        initialize(_imageDev, _maskDev);
    }

    void downsample2x(void)
    {
        int H = imageSize.height();
        int W = imageSize.width();
        int newW = W / 2, newH = H / 2;

        KisPaintDeviceSP imageDev = new KisPaintDevice( cs );
        KisPaintDeviceSP maskDev = new KisPaintDevice( csMask );
        imageDev->writeBytes( imageData.data(), 0, 0, W, H);
        maskDev->writeBytes( maskData.data(), 0, 0, W, H);

        ImageData newImage(newW, newH, cs->pixelSize());
        ImageData newMask(newW, newH, 1);

        KoDummyUpdater updater;
        KisTransformWorker worker(imageDev, 1. / 2., 1. / 2., 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                                  &updater, KisFilterStrategyRegistry::instance()->value("Bicubic"));
        worker.run();

        KisTransformWorker workerMask(maskDev, 1. / 2., 1. / 2., 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                                      &updater, KisFilterStrategyRegistry::instance()->value("Bicubic"));
        workerMask.run();

        imageDev->readBytes( newImage.data(), 0, 0, newW, newH);
        maskDev->readBytes( newMask.data(), 0, 0, newW, newH);
        imageData = std::move(newImage);
        maskData = std::move(newMask);

        for(int i=0; i<imageData.num_elements(); ++i){
            quint8* maskPix = maskData.data()+i*maskData.pixel_size();
            if(*maskPix==MASK_SET){
                for(int k=0; k<imageData.pixel_size(); k++)
                    *(imageData.data()+i*imageData.pixel_size()+k)=0;
            }
            else{
                *maskPix = MASK_CLEAR;
            }
        }
        imageSize = QRect(0, 0, newW, newH);
        int nmasked = countMasked();
        printf("Masked: %d size: %dx%d\n", nmasked, newW, newH);
        maskData.DebugDump("maskData");
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
        return clone;
    }

    int countMasked(void)
    {
        int count = std::count_if(maskData.data(), maskData.data() + maskData.num_elements(), [](quint8 v) {
            return v < ((MASK_SET + MASK_CLEAR / 2));
        });
        return count;
    }

    inline bool isMasked(int x, int y)
    {
        return (*maskData(x, y) < ((MASK_SET + MASK_CLEAR) / 2));
    }

    //returns true if the patch contains a masked pixel
    bool containsMasked(int x, int y, int S)
    {
        for (int dy = -S; dy <= S; ++dy) {
            int ys = y + dy;
            for (int dx = -S; dx <= S; ++dx) {
                int xs = x + dx;
                if (xs < 0 || xs >= imageSize.width())
                    continue;
                if (ys < 0 || ys >= imageSize.height())
                    continue;
                if (isMasked(xs, ys))
                    return true;
            }
        }
        return false;
    }

//    const quint8* getImagePixel(int x, int y) {
//        KisRandomConstAccessorSP it = imageDev->createRandomConstAccessorNG(x, y);
//        return it->oldRawData(); //is this Ok to do?
//    }

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

    inline quint8* getImagePixel( int x, int y ){
        return imageData(x,y);
    }

    inline void setImagePixels(int x, int y, QVector<float>& value)
    {
        cs->fromNormalisedChannelsValue(imageData(x, y), value);
    }

    inline void mixColors( std::vector< quint8* > pixels, std::vector< float > w, float wsum,  quint8* dst ){
        const KoMixColorsOp* mixOp = cs->mixColorsOp();

        size_t n = w.size();
        assert( pixels.size() == n );
        static std::vector< qint16 > weights;
        weights.clear();

        float dif = 0;

        float scale = 255 / (wsum+0.001);

        for( auto& v : w ){
            float v1 = v*scale + dif;
            float v2 = std::round( v1 );
            dif = v1 - v2;
            weights.push_back( v2 );
        }

        mixOp->mixColors(pixels.data(), weights.data(), n, dst );
    }

    inline void setMask(int x, int y, quint8 v)
    {
        *(maskData(x, y)) = v;
    }

    inline int channelCount(void) const
    {
        return cs->channelCount();
    }

    float distance(int x, int y, const MaskedImage& other, int xo, int yo)
    {
        float dsq = 0;
        quint32 nchannels = channelCount();
        quint8* v1 = imageData(x,y);
        quint8* v2 = other.imageData(xo,yo);

        for (quint32 chan = 0; chan < nchannels; chan++) {
            //It's very important not to lose precision in the next line
            //TODO: This code only works for 8bpp data. Refactor to make it universal.
            float v = (float) (*((quint8*)v1+chan)) - (float) (*((quint8*)v2+chan));
            dsq += v * v;
        }
        return dsq;
    }
};

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
    template< typename T> T random_int(T range)
    {
        return rand() % range;
    }

    //compute intial value of the distance term
    void initialize(void)
    {
        for (int y = 0; y < imSize.height(); y++) {
            for (int x = 0; x < imSize.width(); x++) {
                field[x][y].distance = distance(x, y, field[x][y].x, field[x][y].y);

                //if the distance is "infinity", try to find a better link
                int iter = 0;
                const int maxretry = 20;
                while (field[x][y].distance == MAX_DIST && iter < maxretry) {
                    field[x][y].x = random_int(imSize.width() + 1);
                    field[x][y].y = random_int(imSize.height() + 1);
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
        for (int i = 0; i < similarity.size(); i++) {
            float t = (float)i / similarity.size();
            similarity[i] = 0.5 - 0.5 * std::tanh(coef * (t - t_halfmax));
        }
    }

//    void init_similarity_curve(void)
//    {
//        float base[] = {1.0, 0.99, 0.96, 0.83, 0.38, 0.11, 0.02, 0.005, 0.0006, 0.0001, 0 };

//        similarity.resize(MAX_DIST + 1);
//        for (int i = 0; i < similarity.size(); i++) {
//            float t = (float)i / similarity.size();
//            int j = (int)(100 * t);
//            int k = j + 1;
//            float vj = (j < 11) ? base[j] : 0;
//            float vk = (k < 11) ? base[k] : 0;
//            similarity[i] = vj + (100 * t - j) * (vk - vj);
//        }
//    }

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

    typedef boost::multi_array<float, 2> HistArray_type;
    HistArray_type histogram;

public:
    NearestNeighborField(const MaskedImageSP _input, MaskedImageSP _output, int _patchsize) : input(_input), output(_output), patchSize(_patchsize)
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
                field[x][y].x = random_int(imSize.width() + 1);
                field[x][y].y = random_int(imSize.height() + 1);
                field[x][y].distance = MAX_DIST;
            }
        }
        initialize();
    }

    //initialize field from an existing (possibly smaller) nearest neighbor field
    void initialize(const NearestNeighborField& nnf)
    {
        float xscale = imSize.width() / nnf.imSize.width();
        float yscale = imSize.height() / nnf.imSize.height();

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

    //multi-pass NN-field minimization (see "PatchMatch" - page 4)
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
            xp = xpi + random_int(2 * wi) - wi;
            yp = ypi + random_int(2 * wi) - wi;
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
                wsum+=ssdmax;
                int xks = x + dx;
                int yks = y + dy;

                if (xks < 0 || xks >= input->size().width()) {
                    distance+=ssdmax;
                    continue;
                }

                if (yks < 0 || yks >= input->size().height()) {
                    distance+=ssdmax;
                    continue;
                }

                //cannot use masked pixels as a valid source of information
                if (input->isMasked(xks, yks)) {
                    distance+=ssdmax;
                    continue;
                }

                //corresponding pixel in target patch
                int xkt = xp + dx;
                int ykt = yp + dy;
                if (xkt < 0 || xkt >= output->size().width() ) {
                    distance+=ssdmax;
                    continue;
                }
                if (ykt < 0 || ykt >= output->size().height() ) {
                    distance+=ssdmax;
                    continue;
                }

                //cannot use masked pixels as a valid source of information
                if (output->isMasked(xkt, ykt)) {
                    distance+=ssdmax;
                    continue;
                }

                //SSD distance between pixels
                float ssd = input->distance(xks, yks, *output, xkt, ykt);
                //long ssd = input->distance(xks, yks, *input, xkt, ykt);
                distance += ssd;

            }
        }
        return (int)(MAX_DIST * (distance / wsum));
    }

    static MaskedImageSP ExpectationMaximization( KisSharedPtr<NearestNeighborField> TargetToSource, int level, int radius, QList<MaskedImageSP>& pyramid);

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
    Inpaint(KisPaintDeviceSP dev, KisPaintDeviceSP devMask, int _radius)
    {
        initial = new MaskedImage(dev, devMask);
        radius = _radius;
        devCache = dev;
    }
    MaskedImageSP patch(void);
    MaskedImageSP patch_simple(void);
};

class TestClone : public TestUtil::QImageBasedTest
{
public:
    TestClone() : QImageBasedTest("clonetest") {}
    virtual ~TestClone() {}
    void test();
    void testPatchMatch();
private:
    MaskedImageSP patchImage(KisPaintDeviceSP, KisPaintDeviceSP, int radius);
};

MaskedImageSP Inpaint::patch()
{
    MaskedImageSP source = initial->copy();

    pyramid.append(initial);

    QRect size = source->size();

    std::cerr << "countMasked: " <<  source->countMasked() << "\n";
    while ((size.width() > radius) && (size.height() > radius) && source->countMasked() > 0) {
        std::cerr << "countMasked: " <<  source->countMasked() << "\n";
        source->downsample2x();
        source->DebugDump("Pyramid");
        pyramid.append(source->copy());
        size = source->size();
    }
    int maxlevel = pyramid.size();
    std::cerr << "MaxLevel: " <<  maxlevel << "\n";

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

        //Build an upscaled target by EM-like algorithm (see "PatchMatch" - page 6)
        target = NearestNeighborField::ExpectationMaximization(nnf_TargetToSource, level, radius, pyramid);
        target->DebugDump( "target" );
    }
    return target;
}


//EM-Like algorithm (see "PatchMatch" - page 6)
//Returns a float sized target image
MaskedImageSP NearestNeighborField::ExpectationMaximization(NearestNeighborFieldSP nnf_TargetToSource, int level, int radius, QList<MaskedImageSP>& pyramid)
{
    int iterEM = std::min(2*level, 4);
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

        DebugSaver::instance()->debugDumpField("T2S", nnf_TargetToSource->field);

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

    DebugSaver::instance()->debugDumpField("T2S_Final", nnf_TargetToSource->field);
    return newtarget;
}


void NearestNeighborField::ExpectationStep(NearestNeighborFieldSP nnf, MaskedImageSP source, MaskedImageSP target, bool upscale)
{
    //int*** field = nnf->field;
    int R = nnf->patchSize;
    if( upscale )
        R *= 2;

    int H_nnf = nnf->input->size().height();
    int W_nnf = nnf->input->size().width();
    int H_target = target->size().height();
    int W_target = target->size().width();
    int H_source = source->size().height();
    int W_source = source->size().width();

    std::vector< quint8* > pixels;
    std::vector< float > weights;
    pixels.reserve(R*R);
    weights.reserve(R*R);

    for (int x = 0 ; x < W_target ; ++x) {
        for (int y = 0 ; y < H_target; ++y) {
            float wsum = 0;
            pixels.clear();
            weights.clear();


            if( !source->containsMasked(x, y, R) && upscale ){
                //speedup computation by copying parts that are not masked.
                pixels.push_back( source->getImagePixel(x,y) );
                weights.push_back(1.f);
                target->mixColors(pixels, weights, 1.f, target->getImagePixel(x,y));
            } else {
                for (int dx = -R ; dx <= R; ++dx) {
                    for (int dy = -R ; dy <= R ; ++dy) {
                        // xpt,ypt = center pixel of the target patch
                        int xpt = x+dx;
                        int ypt = y+dy;

                        int xst, yst;
                        float w;

                        if( !upscale ){
                            if (xpt < 0 || xpt >= W_nnf || ypt < 0 || ypt >= H_nnf )
                                continue;

                            xst = nnf->field[xpt][ypt].x;
                            yst = nnf->field[xpt][ypt].y;
                            float dp = nnf->field[xpt][ypt].distance;
                            // similarity measure between the two patches
                            w = nnf->similarity[dp];

                        } else {
                            if (xpt < 0 || (xpt/2) >= W_nnf || ypt < 0 || (ypt/2) >= H_nnf )
                                continue;
                            xst = 2*nnf->field[xpt/2][ypt/2].x+(xpt%2);
                            yst = 2*nnf->field[xpt/2][ypt/2].y+(ypt%2);
                            float dp = nnf->field[xpt/2][ypt/2].distance;
                            // similarity measure between the two patches
                            w = nnf->similarity[dp];
                        }

                        int xs = xst - dx;
                        int ys = yst - dy;

                        if (xs < 0 || xs >= W_source || ys < 0 || ys >= H_source )
                            continue;

                        if( source->isMasked(xs, ys))
                            continue;

                        pixels.push_back( source->getImagePixel(xs,ys) );
                        weights.push_back(w);
                        wsum += w;
                    }
                }

                if( wsum < 1 )
                    continue;

                target->mixColors( pixels, weights, wsum, target->getImagePixel(x, y) );
            }
        }
    }
}


MaskedImageSP TestClone::patchImage(KisPaintDeviceSP dev, KisPaintDeviceSP devMask, int radius)
{

    Inpaint inpaint(dev, devMask, radius);
    //return inpaint.patch_simple();

    return inpaint.patch();
}

void TestClone::testPatchMatch()
{
    KisDocument *doc = KisPart::instance()->createDocument();

    //doc->loadNativeFormat("/home/eugening/Projects/patch-inpainting/bungee.kra");
    //doc->loadNativeFormat("/home/eugening/Projects/patch-inpainting/Yosemite_Winter_crop.kra");
    doc->loadNativeFormat("/home/eugening/Projects/patch-inpainting/Yosemite_Winter.kra");

    KisImageSP image = doc->image();
    image->lock();

    KisGroupLayerSP groupLayer = image->rootLayer();
    QObjectList children = groupLayer->children();

    KisNodeSP node = image->root()->firstChild();
    KisPaintDeviceSP mainDev = node->paintDevice();
    const KoColorSpace* mainCS = mainDev->colorSpace();

    KisNodeSP maskNode = node->firstChild();
    KisPaintDeviceSP maskDev = maskNode->paintDevice();

    const KoColorSpace* maskCS = maskDev->colorSpace();

    QRect rect = mainDev->exactBounds();


    KIS_DUMP_DEVICE_2(mainDev, rect, "maindev", "/home/eugening/Projects/img");
    KIS_DUMP_DEVICE_2(maskDev, rect, "maskdev", "/home/eugening/Projects/img");

    MaskedImageSP output = patchImage(mainDev, maskDev, 2);
    if( !output.isNull() ){
        output->toPaintDevice(mainDev);
        KIS_DUMP_DEVICE_2(mainDev, output->size(), "output", "/home/eugening/Projects/Out");
    }
    delete doc;
}


void TestClone::test(void)
{
    KisSurrogateUndoStore *undoStore = new KisSurrogateUndoStore();

    KisImageSP image = createImage(undoStore);
    KisDocument *doc = KisPart::instance()->createDocument();
    doc->setCurrentImage(image);

    image->initialRefreshGraph();

    KisLayerSP layer = new KisPaintLayer(image, "clone", OPACITY_OPAQUE_U8, image->colorSpace());
    image->addNode(layer, image->root());

    KisPaintDeviceSP dev = layer->paintDevice(); //chld->paintDevice(); //
    KisPainter painter(dev);

    delete doc;
}

void KisCloneOpTest::testClone()
{
    TestClone t;
    DebugSaver::instance()->openDebugFile("/home/eugening/Projects/debug.h5");
    //t.test();
    /*QBENCHMARK*/{
        t.testPatchMatch();
    }
}


void KisCloneOpTest::testProjection()
{
    KisDocument *doc = KisPart::instance()->createDocument();
    doc->loadNativeFormat("/home/eugening/Pictures/Krita_Test/Img_20M_3Layer.kra");
//    KisPaintDeviceSP pd = nullptr;

    doc->image()->refreshGraph();
//    QBENCHMARK_ONCE{
//        doc->image()->refreshGraph();
//    }
//    QBENCHMARK_ONCE{
//        pd = doc->image()->projection();
//    }

    //KIS_DUMP_DEVICE_2(proj,proj->exactBounds(),"Img20M","/home/eugening/Projects/Img20M");
    delete doc;
}

QTEST_MAIN(KisCloneOpTest)




