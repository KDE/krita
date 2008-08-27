/*
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _BRACKETING2HDR_H_
#define _BRACKETING2HDR_H_

#include <kparts/plugin.h>

#include <kis_types.h>
#include <kis_iterators_pixel.h>

class KisView2;

class Ui_WdgBracketing2HDR;

/**
 * Template of view plugin
 */
class Bracketing2HDRPlugin : public KParts::Plugin
{
    Q_OBJECT
public:
    enum ResponseType {
        RESPONSE_LINEAR,
        RESPONSE_GAMMA,
        RESPONSE_LOG10
    };
    struct BracketingFrame {
        double exposure; // in ms
        double apexBrightness;
        double aperture;
        int sensitivity; // in iso
        KisImageSP image;
        KisPaintDeviceSP device;
        KisHLineConstIteratorPixel* it;
    };
public:
    Bracketing2HDRPlugin(QObject *parent, const QStringList &);
    virtual ~Bracketing2HDRPlugin();

    /**
     * This function gives access to the response type. This is is an important parameter,
     * as it controls how the light is perceive by the camera.
     * @return the response type
     */
    inline ResponseType responseType() {
        return m_responseType;
    }

    /**
     * @return the bit depth use for the computation, only 8bits is supported for now
     */
    inline int bitDepth() {
        return m_bitDepth;
    }
    /**
     * @return the number of input level, it should be equal to 2 ^ bitDepth()
     */
    inline int numberOfInputLevels() {
        Q_ASSERT(m_numberOfInputLevels == (2 << (bitDepth() - 1)));
        return m_numberOfInputLevels;
    }
private slots:
    void slotNewHDRLayerFromBracketing();
    void slotAddImages();
    void addImage(const QString& filename);

    void computeCameraResponse();
private:
    QList<BracketingFrame> reduceSizeOfFrames(double size, QList<BracketingFrame> originalFrames);
    void computeCameraResponse(QList<BracketingFrame> frames);
    /// Create the HDR paintdevice
    void createHDRPaintDevice(QList<BracketingFrame> frames, KisPaintDeviceSP device);
    /// Normalize the intensity vector such as intensity[numberOfInputLevels()/2] = 1.0
    void normalize(QVector<double>& intensity);
    /// Fill holes in the response
    void fillHoles(QVector<double>& intensity);
    /// Initialize a linear response table
    void computeLinearResponse(QVector<double>& intensity);
    /// Compute the pseudo gaussian weights
    void computePseudoGaussianWeights();
    /// load the images in memory
    bool loadImagesInMemory();
private:
    KisView2 * m_view;
    Ui_WdgBracketing2HDR* m_wdgBracketing2HDR;
    QList<BracketingFrame> m_imagesFrames;
    // Camera response
    QVector<double> m_intensityR, m_intensityG, m_intensityB, m_weights;
    // Options for the calibration
    ResponseType m_responseType;
    int m_bitDepth;
    int m_numberOfInputLevels;
    bool m_cameraResponseIsComputed;
};

#endif // Bracketing2HDRPlugin_H
