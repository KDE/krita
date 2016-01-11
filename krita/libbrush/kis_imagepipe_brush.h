/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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
#ifndef KIS_IMAGEPIPE_BRUSH_
#define KIS_IMAGEPIPE_BRUSH_

#include <QList>
#include <QMap>
#include <QString>

#include "KoResource.h"
#include "kis_gbr_brush.h"
#include "kis_global.h"

class KisPipeBrushParasite;

/**
 * Velocity won't be supported, atm Tilt isn't either,
 * but have chances of implementation
 */
namespace KisParasite
{
enum SelectionMode {
    Constant,
    Incremental,
    Angular,
    Velocity,
    Random,
    Pressure,
    TiltX,
    TiltY
};
}

class BRUSH_EXPORT KisImagePipeBrush : public KisGbrBrush
{

public:
    KisImagePipeBrush(const QString& filename);
    /**
     * Specialized constructor that makes a new pipe brush from a sequence of samesize
     * devices. The fact that it's a vector of a vector, is to support multidimensional
     * brushes (not yet supported!) */
    KisImagePipeBrush(const QString& name, int w, int h,
                      QVector< QVector<KisPaintDevice*> > devices,
                      QVector<KisParasite::SelectionMode> modes);

    virtual ~KisImagePipeBrush();

    virtual bool load();
    virtual bool loadFromDevice(QIODevice *dev);
    virtual bool save();
    virtual bool saveToDevice(QIODevice* dev) const;

    /**
     * @return the next image in the pipe.
    */
    virtual KisFixedPaintDeviceSP paintDevice(const KoColorSpace * colorSpace,
            double scale, double angle,
            const KisPaintInformation& info,
            double subPixelX = 0, double subPixelY = 0) const;

    virtual void setUseColorAsMask(bool useColorAsMask);
    virtual bool hasColor() const;

    virtual enumBrushType brushType() const;

    virtual const KisBoundary* boundary() const;

    virtual bool canPaintFor(const KisPaintInformation& info);

    virtual void makeMaskImage();

    virtual KisImagePipeBrush* clone() const;

    virtual QString defaultFileExtension() const;
    void setAngle(qreal _angle);
    void setScale(qreal _scale);
    void setSpacing(double _spacing);

    quint32 brushIndex(const KisPaintInformation& info) const;
    qint32 maskWidth(double scale, double angle, double subPixelX, double subPixelY, const KisPaintInformation& info) const;
    qint32 maskHeight(double scale, double angle, double subPixelX, double subPixelY, const KisPaintInformation& info) const;

    void notifyStrokeStarted();
    void notifyCachedDabPainted(const KisPaintInformation& info);

    void generateMaskAndApplyMaskOrCreateDab(KisFixedPaintDeviceSP dst, KisBrush::ColoringInformation* coloringInformation,
            double scaleX, double scaleY, double angle,
            const KisPaintInformation& info,
            double subPixelX = 0, double subPixelY = 0, qreal softnessFactor = DEFAULT_SOFTNESS_FACTOR) const;


    QVector<KisGbrBrush*> brushes() const;


    const KisPipeBrushParasite &parasite() const;

    void setParasite(const KisPipeBrushParasite& parasite);
    void setDevices(QVector< QVector<KisPaintDevice*> > devices, int w, int h);

protected:
    void setBrushType(enumBrushType type);
    void setHasColor(bool hasColor);
    /// Will call KisBrush's saveToDevice as well



    KisImagePipeBrush(const KisImagePipeBrush& rhs);

private:
    friend class KisImagePipeBrushTest;

    KisGbrBrush* testingGetCurrentBrush(const KisPaintInformation& info) const;
    void testingSelectNextBrush(const KisPaintInformation& info) const;

    bool initFromData(const QByteArray &data);

private:
    struct Private;
    Private * const m_d;



};

#endif // KIS_IMAGEPIPE_BRUSH_
