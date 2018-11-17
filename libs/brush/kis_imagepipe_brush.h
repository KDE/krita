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

#include <KoResource.h>
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

    ~KisImagePipeBrush() override;

    bool load() override;
    bool loadFromDevice(QIODevice *dev) override;
    bool save() override;
    bool saveToDevice(QIODevice* dev) const override;

    /**
     * @return the next image in the pipe.
    */
    KisFixedPaintDeviceSP paintDevice(const KoColorSpace * colorSpace,
            KisDabShape const&,
            const KisPaintInformation& info,
            double subPixelX = 0, double subPixelY = 0) const override;

    void setUseColorAsMask(bool useColorAsMask) override;
    bool hasColor() const override;

    enumBrushType brushType() const override;

    QString parasiteSelection(); // returns random, constant, etc

    const KisBoundary* boundary() const override;

    bool canPaintFor(const KisPaintInformation& info) override;

    void makeMaskImage() override;

    KisBrushSP clone() const override;

    QString defaultFileExtension() const override;
    void setAngle(qreal _angle) override;
    void setScale(qreal _scale) override;
    void setSpacing(double _spacing) override;

    quint32 brushIndex(const KisPaintInformation& info) const override;
    qint32 maskWidth(KisDabShape const&, double subPixelX, double subPixelY, const KisPaintInformation& info) const override;
    qint32 maskHeight(KisDabShape const&, double subPixelX, double subPixelY, const KisPaintInformation& info) const override;

    void notifyStrokeStarted() override;
    void notifyCachedDabPainted(const KisPaintInformation& info) override;
    void prepareForSeqNo(const KisPaintInformation& info, int seqNo) override;

    void generateMaskAndApplyMaskOrCreateDab(KisFixedPaintDeviceSP dst, KisBrush::ColoringInformation* coloringInformation,
            KisDabShape const&,
            const KisPaintInformation& info,
            double subPixelX = 0, double subPixelY = 0, qreal softnessFactor = DEFAULT_SOFTNESS_FACTOR) const override;


    QVector<KisGbrBrushSP> brushes() const;

    const KisPipeBrushParasite &parasite() const;

    void setParasite(const KisPipeBrushParasite& parasite);
    void setDevices(QVector< QVector<KisPaintDevice*> > devices, int w, int h);

protected:
    void setBrushType(enumBrushType type) override;
    void setHasColor(bool hasColor) override;
    /// Will call KisBrush's saveToDevice as well

    KisImagePipeBrush(const KisImagePipeBrush& rhs);

private:
    friend class KisImagePipeBrushTest;

    KisGbrBrushSP testingGetCurrentBrush(const KisPaintInformation& info) const;
    void testingSelectNextBrush(const KisPaintInformation& info) const;

    bool initFromData(const QByteArray &data);

    QString parasiteSelectionString; // incremental, random, etc.

private:
    struct Private;
    Private * const m_d;
};

typedef QSharedPointer<KisImagePipeBrush> KisImagePipeBrushSP;

#endif // KIS_IMAGEPIPE_BRUSH_
