/*
 * matching.cpp -- Part of Krita
 *
 * Copyright (c) 2005-2006 Cyrille Berger (cberger@cberger.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#include "harris_detector.h"


#include <map>

#include <kis_debug.h>

#include <kis_convolution_painter.h>
#include <kis_generic_colorspace.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_transaction.h>
#include <kis_paint_device.h>

#define WINDOW_SIZE 9
#define THRESHOLD_CLOSENESS 0.7

#define ABS(x) ((x) < 0 ? -(x) : (x))

#define DERIVATION_SIGMA 1.0
#define CONVOLUTION_SIGMA 2.0
#define THRESHOLD_LAMBDA 0.0
#define FEATURES_QUANTITY 2000

typedef KisGenericColorSpace<float, 8> KisEightFloatColorSpace;

#define DERIVATION_SIGMA 1.0

#define TOPTOP 0
#define LEFTLEFT 0
#define TOP 1
#define LEFT 1
#define CENTER 2
#define BOTTOM 3
#define RIGHT 3
#define BOTTOMBOTTOM 4
#define RIGHTRIGHT 4

#define INFO_HDIFF 0
#define INFO_HADD 1
#define INFO_XX 0
#define INFO_YY 1
#define INFO_XY 2
#define INFO_X 3
#define INFO_Y 4
#define INFO_INTENSITY 5
#define INFO_HIGH 6
#define INFO_LOW 7

#define ZNCC_HALF_WINDOW_SIZE 5
#define ZNCC_WINDOW_SIZE (2*ZNCC_HALF_WINDOW_SIZE + 1)

inline double pow2(double a)
{
    return a * a;
}

class HarrisPoint : public KisInterestPoint
{
public:
    HarrisPoint(double x, double y, double intensity, double high, double low, KisPaintDeviceSP device) : KisInterestPoint(x, y), m_device(device), m_intensity(intensity), m_high(high), m_low(low) {
    }
    virtual double fastCompare(const KisInterestPoint* ip) const {
        const HarrisPoint* ip2 = dynamic_cast<const HarrisPoint*>(ip);
        Q_ASSERT(ip2);
        double score = 0.0;
        if (intensity() > ip2->intensity()) {
            score += ip2->intensity() / intensity();
        } else {
            score += intensity() / ip2->intensity();
        }
        if (high() > ip2->high()) {
            score += ip2->high() / high();
        } else {
            score += high() / ip2->high();
        }
        if (low() > ip2->low()) {
            score += ip2->low() / low();
        } else {
            score += low() / ip2->low();
        }
        return score *(1. / 3.);
    }
    virtual double compare(const KisInterestPoint* ip) const {
        const HarrisPoint* ip2 = dynamic_cast<const HarrisPoint*>(ip);
        Q_ASSERT(ip2);
        // Compute the mean
        double mean1 = 0.0;
        double mean2 = 0.0;
        int zncc_count = 0;
        {
            KisHLineConstIterator it1 = device()->createHLineConstIterator(x() - ZNCC_HALF_WINDOW_SIZE, y() - ZNCC_HALF_WINDOW_SIZE, ZNCC_WINDOW_SIZE);
            KisHLineConstIterator it2 = ip2->device()->createHLineConstIterator(ip2->x() - ZNCC_HALF_WINDOW_SIZE, ip2->y() - ZNCC_HALF_WINDOW_SIZE, ZNCC_WINDOW_SIZE);
            for (int j = 0; j < ZNCC_WINDOW_SIZE; j++) {
                while (!it1.isDone()) {
                    mean1 += *it1.oldRawData();
                    mean2 += *it2.oldRawData();
                    ++zncc_count;
                    ++it1;
                    ++it2;
                }
                it1.nextRow();
                it2.nextRow();
            }
            mean1 /= zncc_count;
            mean2 /= zncc_count;
        }
        // Compute sigma and zncc sum
        double sigma1 = 0.;
        double sigma2 = 0.;
        double zncc_sum = 0.;
        {
            KisHLineConstIterator it1 = device()->createHLineConstIterator(x() - ZNCC_HALF_WINDOW_SIZE, y() - ZNCC_HALF_WINDOW_SIZE, ZNCC_WINDOW_SIZE);
            KisHLineConstIterator it2 = ip2->device()->createHLineConstIterator(ip2->x() - ZNCC_HALF_WINDOW_SIZE, ip2->y() - ZNCC_HALF_WINDOW_SIZE, ZNCC_WINDOW_SIZE);
            for (int j = 0; j < ZNCC_WINDOW_SIZE; j++) {
                while (!it1.isDone()) {
                    double v1 = *it1.oldRawData();
                    double v2 = *it2.oldRawData();
                    sigma1 += pow2(v1 - mean1);
                    sigma2 += pow2(v2 - mean2);
                    zncc_sum += v1 * v2;
                    ++it1;
                    ++it2;
                }
                it1.nextRow();
                it2.nextRow();
            }
        }
        sigma1 = sqrt(sigma1 / zncc_count);
        sigma2 = sqrt(sigma2 / zncc_count);
        zncc_sum = zncc_sum / zncc_count - mean1 * mean2;

        return zncc_sum / (sigma1 * sigma2);
    }
    inline double low() const {
        return m_low;
    }
    inline double intensity() const {
        return m_intensity;
    }
    inline double high() const {
        return m_high;
    }
    virtual QString toString() const {
        return QString("low = %1 high = %2 intensity = %3").arg(low()).arg(high()).arg(intensity());
    }
    inline KisPaintDeviceSP device() const {
        return m_device;
    }
private:
    KisPaintDeviceSP m_device;
    double m_intensity, m_high, m_low;
};

class HarrisPoints
{
public:
    HarrisPoints(int hzones, int vzones, int width, int height, int nbPoints) : m_hzones(hzones), m_vzones(vzones), m_width(width), m_height(height), m_nbPoints(nbPoints / (hzones * vzones)), m_zoneWidth(m_width / m_hzones), m_zoneHeight(m_height / m_hzones), m_zones(new lInterestPoints[ m_hzones * m_vzones ]) {
    }
    ~HarrisPoints() {
//             delete[] m_zones; // Crashes, why ?
    }
    lInterestPoints points() const {
        lInterestPoints points;
        for (int i = 0; i < m_hzones * m_vzones; ++i) {
            points += m_zones[i];
        }
        return points;
    }
    void instertPoint(HarrisPoint* hp) {
        int u = (int)(hp->x() / m_zoneWidth);
        int v = (int)(hp->y() / m_zoneHeight);
        lInterestPoints &zone = m_zones[ u * m_hzones + v ];
        if (zone.empty()) {
            zone.push_back(hp);
        } else {
            if (zone.size() >= m_nbPoints && hp->low() > static_cast<HarrisPoint*>(zone.back())->low()) { // remove last element, the totalNumber stay equal to FEATURES_QUANTITY
                zone.pop_back();
            }
            if (zone.size() != m_nbPoints) {
                lInterestPoints::iterator it;
                if (hp->low() < static_cast<HarrisPoint*>(zone.back())->low()) {
                    zone.push_back(hp);
                } else {
                    // insert the new corner at its right place
                    bool inserted = false;
                    for (it = zone.begin(); it != zone.end(); it++) {
                        if (hp->low() >= static_cast<HarrisPoint*>(*it)->low()) {
                            //dbgPlugins <<"insert point";
                            zone.insert(it, hp);
                            inserted = true;
                            break;
                        }
                    }
                    if (!inserted) delete hp;
                }
            } else { // hp wasn't added to the list, remove it
                delete hp;
            }
        }
    }
private:
    int m_hzones, m_vzones, m_width, m_height, m_nbPoints, m_zoneWidth, m_zoneHeight;
    lInterestPoints* m_zones;
};

lInterestPoints HarrisPointDetector::computeInterestPoints(KisPaintDeviceSP device, const QRect& rect)
// lHarrisPoints computeHarrisPoints(KisPaintDeviceSP device, QRect rect)
{
    dbgPlugins << "Compute Harris points on the rect :" << rect;
    Q_ASSERT(device->colorSpace()->id() == "GRAYA");
    lInterestPoints points;

    // Compute the derivatives
    KisEightFloatColorSpace* floatCs = new KisEightFloatColorSpace();
    KisPaintDeviceSP infoDevice = new KisPaintDevice(floatCs, "infoDevice");

    float g_var = DERIVATION_SIGMA * DERIVATION_SIGMA ;
    float sqrt2pi = sqrt(2 * M_PI);
    float beta0 = 1 / (sqrt2pi * DERIVATION_SIGMA);

    float beta1 = beta0 * exp(-1 / (2 * g_var));
    float beta2 = beta0 * exp(-4 / (2 * g_var));

    float alpha1 = beta1 / g_var;
    float alpha2 = 2 * beta2 / g_var;



    {
        KisHLineConstIteratorPixel hitDevice = device->createHLineConstIterator(rect.left(), rect.top() + 2, rect.width() - 4);
        KisHLineIteratorPixel hitinfoDevice = infoDevice-> createHLineIterator(rect.left() + 2, 2, rect.width());

        quint8 pixelvalue[5];

        dbgPlugins << " Compute the derivatives";
        dbgPlugins << "  horizontal derivatives";
        /* Horizontal computation of derivatives */
        for (int y = rect.top() + 2; y < rect.bottom() - 2; y++) {
            pixelvalue[LEFTLEFT] = *hitDevice.rawData();
            ++hitDevice;
            pixelvalue[LEFT] = *hitDevice.rawData();
            ++hitDevice;
            pixelvalue[CENTER] = *hitDevice.rawData();
            ++hitDevice;
            pixelvalue[RIGHT] = *hitDevice.rawData();
            ++hitDevice;
            while (!hitDevice.isDone()) {
                pixelvalue[RIGHTRIGHT] = *hitDevice.rawData();
                float* infoValues = reinterpret_cast<float*>(hitinfoDevice.rawData());
                infoValues[INFO_HDIFF] = alpha1 * (pixelvalue[LEFT] - pixelvalue[RIGHT]) + alpha2 * (pixelvalue[LEFTLEFT] - pixelvalue[RIGHTRIGHT]);
                infoValues[INFO_HADD] = beta0 * (pixelvalue[CENTER]) + beta1 * (pixelvalue[LEFT] + pixelvalue[RIGHT]) + beta2 * (pixelvalue[LEFTLEFT] + pixelvalue[RIGHTRIGHT]);
                infoValues[INFO_INTENSITY] = pixelvalue[CENTER];
//                     dbgPlugins << hitDevice.x() <<"" << hitDevice.y() <<"" << infoValues[INFO_HDIFF] <<"" << infoValues[INFO_HADD] <<"" << (int)pixelvalue[CENTER] <<"" << infoValues[INFO_INTENSITY];
                memmove(pixelvalue, pixelvalue + 1, 4*sizeof(quint8));
                ++hitDevice;
                ++hitinfoDevice;
            }
            hitDevice.nextRow();
            hitinfoDevice.nextRow();
        }
    }

    KisTransaction a("", infoDevice);
    {
        KisVLineConstIteratorPixel vitinfoDeviceRead = infoDevice-> createVLineConstIterator(rect.left() + 4, rect.top(), rect.height());
        KisVLineIteratorPixel vitinfoDevice = infoDevice-> createVLineIterator(rect.left() + 4, rect.top() + 2, rect.height() - 2);

        float hdiffValue[5];
        float haddValue[5];
        dbgPlugins << "  vertical derivatives";
        /* Vertical computation of derivatives */
        for (int x = rect.left() + 4; x < rect.right() - 4; x++) {
            const float* infoValue = reinterpret_cast<const float*>(vitinfoDeviceRead.oldRawData());
            hdiffValue[TOPTOP] = infoValue[INFO_HDIFF];
            haddValue[TOPTOP] = infoValue[INFO_HADD];
            ++vitinfoDeviceRead;
            infoValue = reinterpret_cast<const float*>(vitinfoDeviceRead.oldRawData());
            hdiffValue[TOP] = infoValue[INFO_HDIFF];
            haddValue[TOP] = infoValue[INFO_HADD];
            ++vitinfoDeviceRead;
            infoValue = reinterpret_cast<const float*>(vitinfoDeviceRead.oldRawData());
            hdiffValue[CENTER] = infoValue[INFO_HDIFF];
            haddValue[CENTER] = infoValue[INFO_HADD];
            ++vitinfoDeviceRead;
            infoValue = reinterpret_cast<const float*>(vitinfoDeviceRead.oldRawData());
            hdiffValue[RIGHT] = infoValue[INFO_HDIFF];
            haddValue[RIGHT] = infoValue[INFO_HADD];
            ++vitinfoDeviceRead;

            while (!vitinfoDevice.isDone()) {
                infoValue = reinterpret_cast<const float*>(vitinfoDeviceRead.oldRawData());
                hdiffValue[RIGHTRIGHT] = infoValue[INFO_HDIFF];
                haddValue[RIGHTRIGHT] = infoValue[INFO_HADD];

                float c_grdy = beta0 * hdiffValue[ CENTER ] + beta1 * (hdiffValue[ TOP ] + hdiffValue[ BOTTOM ]) + beta2 * (hdiffValue[ TOPTOP ] + hdiffValue[ BOTTOMBOTTOM ]);
                float c_grdx = alpha1 * (haddValue[ TOP ] - haddValue[ BOTTOM ]) + alpha2 * (haddValue[ TOPTOP ] - haddValue[ BOTTOMBOTTOM ]);

                float* infoValueDst = reinterpret_cast<float*>(vitinfoDevice.rawData());
                infoValueDst[ INFO_XX ] = c_grdx * c_grdx;
                infoValueDst[ INFO_YY ] = c_grdy * c_grdy;
                infoValueDst[ INFO_X ] = c_grdx;
                infoValueDst[ INFO_Y ] = c_grdy;
                infoValueDst[ INFO_XY ] = c_grdx * c_grdy;

                memmove(hdiffValue, hdiffValue + 1, 4 * sizeof(float));
                memmove(haddValue , haddValue + 1 , 4 * sizeof(float));
                ++vitinfoDeviceRead;
                ++vitinfoDevice;
            }
            vitinfoDeviceRead.nextCol();
            vitinfoDevice.nextCol();
        }
    }

    // Apply a blur

    // Apply a blur
    KisTransaction("", infoDevice);

#if 1
    {
        KisHLineConstIteratorPixel hitDevice = infoDevice->createHLineConstIterator(rect.left(), rect.top() + 2, rect.width() - 4);
        KisHLineIteratorPixel hitinfoDevice = infoDevice-> createHLineIterator(rect.left() + 2, rect.top() + 2, rect.width() - 4);

        float pixelvalue[6][6];

        dbgPlugins << " Compute the blur";
        dbgPlugins << "  horizontal blur";
        /* Horizontal computation of derivatives */
        for (int y = rect.top() + 2; y < rect.bottom() - 2; y++) {
            memcpy(pixelvalue[LEFTLEFT], hitDevice.rawData(), 6*sizeof(float));
            ++hitDevice;
            memcpy(pixelvalue[LEFT], hitDevice.rawData(), 6*sizeof(float));
            ++hitDevice;
            memcpy(pixelvalue[CENTER], hitDevice.rawData(), 6*sizeof(float));
            ++hitDevice;
            memcpy(pixelvalue[RIGHT], hitDevice.rawData(), 6*sizeof(float));
            ++hitDevice;
            while (!hitDevice.isDone()) {
                memcpy(pixelvalue[RIGHTRIGHT], hitDevice.rawData(), 6*sizeof(float));
                float* infoValues = reinterpret_cast<float*>(hitinfoDevice.rawData());
                for (int i = 0; i < 5; i++) {
                    infoValues[i] = beta0 *  pixelvalue[CENTER][i] + beta1 * (pixelvalue[LEFT][i] + pixelvalue[RIGHT][i]) + beta2 * (pixelvalue[LEFTLEFT][i] + pixelvalue[RIGHTRIGHT][i]);
//                   infoValues[i] =2 *  pixelvalue[CENTER][i] + ( pixelvalue[LEFT][i] + pixelvalue[RIGHT][i] );
                }
//                 memmove(pixelvalue, pixelvalue + 1, 4*sizeof(float[6]));
                for (int i = 0; i < 5; i++) {
                    memcpy(pixelvalue[i], pixelvalue[i+1], 6*sizeof(float));
                }
                ++hitDevice;
                ++hitinfoDevice;
            }
            hitDevice.nextRow();
            hitinfoDevice.nextRow();
        }
    }
    KisTransaction b("", infoDevice);
    {
        KisVLineConstIteratorPixel vitinfoDeviceRead = infoDevice-> createVLineConstIterator(rect.left() + 4, rect.top(), rect.height());
        KisVLineIteratorPixel vitinfoDevice = infoDevice-> createVLineIterator(rect.left() + 4, rect.top() + 2, rect.height() - 4);

        float infoValue[6][6];
        dbgPlugins << "  vertical blur";
        /* Vertical computation of derivatives */
        for (int x = rect.left() + 4; x < rect.right() - 4; x++) {
            memcpy(infoValue[TOPTOP], vitinfoDeviceRead.oldRawData(), 6*sizeof(float));
            ++vitinfoDeviceRead;
            memcpy(infoValue[TOP], vitinfoDeviceRead.oldRawData(), 6*sizeof(float));
            ++vitinfoDeviceRead;
            memcpy(infoValue[CENTER], vitinfoDeviceRead.oldRawData(), 6*sizeof(float));
            ++vitinfoDeviceRead;
            memcpy(infoValue[BOTTOM], vitinfoDeviceRead.oldRawData(), 6*sizeof(float));
            ++vitinfoDeviceRead;
            while (!vitinfoDevice.isDone()) {
                memcpy(infoValue[BOTTOMBOTTOM], vitinfoDeviceRead.oldRawData(), 6*sizeof(float));
                float* dst = reinterpret_cast<float*>(vitinfoDevice.rawData());
                for (int i = 0; i < 5; i++) {
                    dst[i] =  beta0 * infoValue[CENTER][i] + beta1 * (infoValue[BOTTOM][i] + infoValue[TOP][i]) + beta2 * (infoValue[TOPTOP][i] + infoValue[BOTTOMBOTTOM][i]);
//                   dst[i] =  (2*infoValue[CENTER][i] + ( infoValue[BOTTOM][i] + infoValue[TOP][i] )) / 16;
                }
//                 memmove(infoValue, infoValue + 1, 4*sizeof(float[6]));
                for (int i = 0; i < 5; i++) {
                    memcpy(infoValue[i], infoValue[i+1], 6*sizeof(float));
                }
                ++vitinfoDeviceRead;
                ++vitinfoDevice;
            }
            vitinfoDeviceRead.nextCol();
            vitinfoDevice.nextCol();
        }
    }
#endif

#if 0
    KisTransaction("", infoDevice);
    dbgPlugins << " Blur";
    {
        // Compute the blur mask
        KisAutobrushShape* kas = new KisAutobrushCircleShape(5, 5, 2, 2);

        QImage mask;
        kas->createBrush(&mask);
        KisKernelSP kernel = KisKernel::fromQImage(mask);
        // Apply the convolution to xxDevice
        KisConvolutionPainter infoDevicePainter(infoDevice);
        infoDevicePainter.beginTransaction("bouuh");
        infoDevicePainter.applyMatrix(kernel, 2, 2, rect.width() - 4, rect.height() - 4, BORDER_REPEAT);
        delete kas;
    }
#endif
    dbgPlugins << " compute curvatures";
    // Compute the curvatures
    {
        KisRectIteratorPixel vitinfoDeviceRect = infoDevice->createRectIterator(2, 0, rect.width() - 2, rect.height() - 2);
        for (; !vitinfoDeviceRect.isDone(); ++vitinfoDeviceRect) {
            float* infoValue = reinterpret_cast<float*>(vitinfoDeviceRect.rawData());

            float det = infoValue[INFO_XX] * infoValue[INFO_YY] - infoValue[INFO_XY] * infoValue[INFO_XY];
            float trace = infoValue[INFO_XX] + infoValue[INFO_YY];
            float temp = sqrt(trace * trace - 4 * det);

            infoValue[ INFO_HIGH ] = 0.5 * (trace + temp);
            infoValue[ INFO_LOW  ] = 0.5 * (trace - temp);
            if (infoValue[ INFO_HIGH ] < infoValue[ INFO_LOW  ]) {
                float a = infoValue[ INFO_HIGH ];
                infoValue[ INFO_HIGH ] = infoValue[ INFO_LOW ];
                infoValue[ INFO_LOW ] = a;
            }
//                 dbgPlugins << vitinfoDeviceRect.x() <<"" << vitinfoDeviceRect.y() <<"" << infoValue[INFO_XX] <<"" << infoValue[INFO_YY]  <<"" << infoValue[INFO_XY] <<"" << infoValue[INFO_HIGH] <<"" << infoValue[INFO_LOW] <<"" << trace <<"" << temp <<"" << det;
        }
    }
    HarrisPoints zones(5, 5, rect.width(), rect.height(), FEATURES_QUANTITY);
    // Detect Harris Points
    {
        int margin = 8;
        KisHLineIterator hitinfoDevice = infoDevice-> createHLineIterator(margin, margin, rect.width() - 2 * margin);
        for (int y = margin + rect.top(); y < rect.bottom() - margin; y++) {
            for (int x = margin + rect.left(); x < rect.right() - margin; x++, ++hitinfoDevice) {
                float* infoValue = reinterpret_cast<float*>(hitinfoDevice.rawData());
                float low = infoValue[ INFO_LOW ];
//                     dbgPlugins << low;
                if (low > THRESHOLD_LAMBDA) {
                    KisRectIteratorPixel vitinfoDeviceRect = infoDevice->createRectIterator(x - 1, y - 1, 3, 3);
                    bool greater = true;
                    for (; !vitinfoDeviceRect.isDone(); ++vitinfoDeviceRect) {
                        if (reinterpret_cast<float*>(vitinfoDeviceRect.rawData())[ INFO_LOW ] > low) {
                            greater = false;
                            break;
                        }
                    }
                    if (greater) {
//                             dbgPlugins <<"new point";
                        HarrisPoint* hp = new HarrisPoint(x, y, infoValue[INFO_INTENSITY], infoValue[INFO_HIGH], infoValue[INFO_LOW], device);
#if 0
                        points.push_back(hp);
#endif
#if 0
                        if (points.empty()) {
                            points.push_back(hp);
                        } else {
                            if (points.size() >= FEATURES_QUANTITY && hp->low() > static_cast<HarrisPoint*>(points.back())->low()) { // remove last element, the totalNumber stay equal to FEATURES_QUANTITY
                                points.pop_back();
                            }
                            if (points.size() != FEATURES_QUANTITY) {
                                lInterestPoints::iterator it;
                                if (hp->low() < static_cast<HarrisPoint*>(points.back())->low()) {
                                    points.push_back(hp);
                                } else {
                                    // insert the new corner at his right place
                                    bool inserted = false;
                                    for (it = points.begin(); it != points.end(); it++) {
                                        if (hp->low() >= static_cast<HarrisPoint*>(*it)->low()) {
                                            //                                             dbgPlugins <<"insert point";
                                            points.insert(it, hp);
                                            inserted = true;
                                            break;
                                        }
                                    }
                                    if (!inserted) delete hp;
                                }
                            } else { // hp wasn't added to the list, remove it
                                delete hp;
                            }
                        }
#endif
#if 1
                        zones.instertPoint(hp);
#endif
                    }
                }
            }
            hitinfoDevice.nextRow();
        }
    }
    points = zones.points();
    dbgPlugins << "Harris detector has found :" << points.size() << " harris points";

    delete floatCs;
    return points;
}
