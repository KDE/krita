/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_BRUSH_
#define KIS_BRUSH_

#include <q3cstring.h>
#include <qstring.h>
#include <qsize.h>
#include <qimage.h>
#include <q3valuevector.h>
//Added by qt3to4:
#include <QPixmap>

#include <kio/job.h>

#include "kis_resource.h"
#include "kis_types.h"
#include "kis_point.h"
#include "kis_alpha_mask.h"
#include "koffice_export.h"
#include "kis_boundary.h"
#include "kis_paintop.h"

class QPoint;
class QPixmap;
class KisBoundary;
class KisColorSpace;
class QIODevice;

enum enumBrushType {
    INVALID,
    MASK,
    IMAGE,
    PIPE_MASK,
    PIPE_IMAGE,
    AIRBRUSH
};

class KRITACORE_EXPORT KisBrush : public KisResource {
    typedef KisResource super;
    Q_OBJECT

public:
    /// Construct brush to load filename later as brush
    KisBrush(const QString& filename);
    /// Load brush from the specified data, at position dataPos, and set the filename
    KisBrush(const QString& filename,
         const QByteArray & data,
         qint32 & dataPos);
    /// Load brush from the specified paint device, in the specified region
    KisBrush(KisPaintDevice* image, int x, int y, int w, int h);
    /// Load brush as a copy from the specified QImage (handy when you need to copy a brush!)
    KisBrush(const QImage& image, const QString& name = QString(""));

    virtual ~KisBrush();

    virtual bool load();
    /// synchronous, doesn't emit any signal (none defined!)
    virtual bool save();
    virtual QImage img();
    virtual bool saveToDevice(QIODevice* dev) const;

    /**
       @return a mask computed from the grey-level values of the
       pixels in the brush.
    */
    virtual KisAlphaMaskSP mask(const KisPaintInformation& info,
                                double subPixelX = 0, double subPixelY = 0) const;
    // XXX: return non-tiled simple buffer
    virtual KisPaintDeviceSP image(KisColorSpace * colorSpace, const KisPaintInformation& info,
                             double subPixelX = 0, double subPixelY = 0) const;

    void setHotSpot(KisPoint);
    KisPoint hotSpot(const KisPaintInformation& info = KisPaintInformation()) const;

    void setSpacing(double s) { m_spacing = s; }
    double spacing() const { return m_spacing; }
    double xSpacing(double pressure = PRESSURE_DEFAULT) const;
    double ySpacing(double pressure = PRESSURE_DEFAULT) const;

    // Dimensions in pixels of the mask/image at a given pressure.
    qint32 maskWidth(const KisPaintInformation& info) const;
    qint32 maskHeight(const KisPaintInformation& info) const;

    virtual void setUseColorAsMask(bool useColorAsMask) { m_useColorAsMask = useColorAsMask; }
    virtual bool useColorAsMask() const { return m_useColorAsMask; }
    virtual bool hasColor() const;

    virtual void makeMaskImage();
    qint32 width() const;
    qint32 height() const;

    virtual enumBrushType brushType() const;

    //QImage outline(double pressure = PRESSURE_DEFAULT);
    virtual KisBoundary boundary();

    /**
     * Returns true if this brush can return something useful for the info. This is used
     * by Pipe Brushes that can't paint sometimes
     **/
    virtual bool canPaintFor(const KisPaintInformation& /*info*/) { return true; }

    virtual KisBrush* clone() const;

protected:
    void setWidth(qint32 w);
    void setHeight(qint32 h);
    void setImage(const QImage& img);
    void setBrushType(enumBrushType type) { m_brushType = type; };
    static double scaleForPressure(double pressure);

private:
    class ScaledBrush {
    public:
        ScaledBrush();
        ScaledBrush(KisAlphaMaskSP scaledMask, const QImage& scaledImage, double scale, double xScale, double yScale);

        double scale() const { return m_scale; }
        double xScale() const { return m_xScale; }
        double yScale() const { return m_yScale; }
        KisAlphaMaskSP mask() const { return m_mask; }
        QImage image() const { return m_image; }

    private:
        KisAlphaMaskSP m_mask;
        QImage m_image;
        double m_scale;
        double m_xScale;
        double m_yScale;
    };


    bool init();
    bool initFromPaintDev(KisPaintDevice* image, int x, int y, int w, int h);
    void createScaledBrushes() const;

    KisAlphaMaskSP scaleMask(const ScaledBrush *srcBrush, double scale, double subPixelX, double subPixelY) const;
    QImage scaleImage(const ScaledBrush *srcBrush, double scale, double subPixelX, double subPixelY) const;

    static QImage scaleImage(const QImage& srcImage, int width, int height);
    static QImage interpolate(const QImage& image1, const QImage& image2, double t);

    static KisAlphaMaskSP scaleSinglePixelMask(double scale, quint8 maskValue, double subPixelX, double subPixelY);
    static QImage scaleSinglePixelImage(double scale, QRgb pixel, double subPixelX, double subPixelY);

    // Find the scaled brush(es) nearest to the given scale.
    void findScaledBrushes(double scale, const ScaledBrush **aboveBrush, const ScaledBrush **belowBrush) const;

    // Initialize our boundary
    void generateBoundary();

    QByteArray m_data;
    bool m_ownData;
    KisPoint m_hotSpot;
    double m_spacing;
    bool m_useColorAsMask;
    bool m_hasColor;
    QImage m_img;
    mutable Q3ValueVector<ScaledBrush> m_scaledBrushes;

    qint32 m_width;
    qint32 m_height;

    quint32 m_header_size;  /*  header_size = sizeof (BrushHeader) + brush name  */
    quint32 m_version;      /*  brush file version #  */
    quint32 m_bytes;        /*  depth of brush in bytes */
    quint32 m_magic_number; /*  GIMP brush magic number  */

    enumBrushType m_brushType;

    KisBoundary* m_boundary;

};
#endif // KIS_BRUSH_

