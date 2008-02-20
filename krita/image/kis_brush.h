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

#include <QImage>
#include <QVector>

#include "kis_types.h"
#include "kis_shared.h"
#include "kis_paint_information.h"

#include "KoResource.h"

#include "krita_export.h"

class QString;
class QPoint;
class QIODevice;

class KoColor;
class KoColorSpace;

class KisBoundary;

enum enumBrushType {
    INVALID,
    MASK,
    IMAGE,
    PIPE_MASK,
    PIPE_IMAGE,
    AIRBRUSH
};

class KisBrush;

/**
 * A brush factory can create a new brush instance based
 * on a filename and a brush id or just a brush id.
 */
class KRITAIMAGE_EXPORT KisBrushFactory : public KisShared {

public:

    KisBrushFactory() {}
    virtual ~KisBrushFactory() {}

    virtual KisBrush * createBrush() const = 0;
    virtual KisBrush * createBrush( const QString & fileName ) const = 0;
    virtual KisBrush * createBrush( const QString& filename,
                                    const QByteArray & data,
                                    qint32 & dataPos) const = 0;

    /// Load brush from the specified paint device, in the specified region
    virtual KisBrush * createBrush (KisPaintDeviceSP image, int x, int y, int w, int h) const = 0;

    /// Load brush as a copy from the specified QImage (handy when you need to copy a brush!)
    virtual KisBrush* createBrush(const QImage& image, const QString& name = QString("")) const = 0;

};


class KRITAIMAGE_EXPORT KisBrush : public KoResource {

  class ScaledBrush;

    Q_OBJECT
    
protected:

    class ColoringInformation {
    public:
        virtual ~ColoringInformation();
        virtual const quint8* color() const = 0;
        virtual void nextColumn() = 0;
        virtual void nextRow() = 0;
    };
    
    class PlainColoringInformation : public ColoringInformation {
    public:
        PlainColoringInformation(const quint8* color);
        virtual ~PlainColoringInformation();
        virtual const quint8* color() const ;
        virtual void nextColumn();
        virtual void nextRow();
    private:
        const quint8* m_color;
    };
    
    class PaintDeviceColoringInformation : public ColoringInformation {
    public:
        PaintDeviceColoringInformation(const KisPaintDeviceSP source, int width);
        virtual ~PaintDeviceColoringInformation();
        virtual const quint8* color() const ;
        virtual void nextColumn();
        virtual void nextRow();
    private:
        const KisPaintDeviceSP m_source;
        KisHLineConstIteratorPixel* m_iterator;
    };
    
public:

    /// Construct brush to load filename later as brush
    KisBrush(const QString& filename);

    /// Load brush from the specified data, at position dataPos, and set the filename
    KisBrush(const QString& filename,
             const QByteArray & data,
             qint32 & dataPos);

    /// Load brush from the specified paint device, in the specified region
    KisBrush(KisPaintDeviceSP image, int x, int y, int w, int h);

    /// Load brush as a copy from the specified QImage (handy when you need to copy a brush!)
    KisBrush(const QImage& image, const QString& name = QString(""));

    virtual ~KisBrush();

    virtual bool load();
    /// synchronous, doesn't emit any signal (none defined!)
    virtual bool save();
    /**
     * @return a preview of the brush
     */
    virtual QImage img() const;
    /**
     * save the content of this brush to an IO device
     */
    virtual bool saveToDevice(QIODevice* dev) const;

    /**
     * 
     * @param dst the destination that will be draw on the image, and this function
     *            will edit its alpha channel
     * @param src coloring information that will be copied on the dab, it can be null
     * @param scale a scale applied on the alpha mask
     * @param angle a rotation applied on the alpha mask
     * @param info the painting information (this is only and should only be used by
     *             KisImagePipeBrush and only to be backward compatible with the Gimp,
     *             KisImagePipeBrush is ignoring scale and angle information)
     * @param subPixelX sub position of the brush (contained between 0.0 and 1.0)
     * @param subPixelY sub position of the brush (contained between 0.0 and 1.0)
     * 
     * @return a mask computed from the grey-level values of the
     * pixels in the brush.
     */
    virtual void generateMask(KisPaintDeviceSP dst, ColoringInformation* src, double scaleX, double scaleY, double angle, const KisPaintInformation& info = KisPaintInformation(), double subPixelX = 0, double subPixelY = 0) const;
    
    void mask(KisPaintDeviceSP dst, double scaleX, double scaleY, double angle, const KisPaintInformation& info = KisPaintInformation(), double subPixelX = 0, double subPixelY = 0) const;

    void mask(KisPaintDeviceSP dst, const KoColor& color, double scaleX, double scaleY, double angle, const KisPaintInformation& info = KisPaintInformation(), double subPixelX = 0, double subPixelY = 0) const;

    void mask(KisPaintDeviceSP dst, const KisPaintDeviceSP src, double scaleX, double scaleY, double angle, const KisPaintInformation& info = KisPaintInformation(), double subPixelX = 0, double subPixelY = 0) const;

    // XXX: return non-tiled simple buffer
    virtual KisPaintDeviceSP image(const KoColorSpace * colorSpace, double scale, double angle, const KisPaintInformation& info, double subPixelX = 0, double subPixelY = 0) const;

    void setHotSpot(QPointF);

    QPointF hotSpot(double scaleX, double scaleY ) const;

    /**
     * Change the spacing of the brush.
     * @param spacing a spacing of 1.0 means that strokes will be seperated from one time the size
     *                of the brush.
     */
    void setSpacing(double spacing);

    /**
     * @return the spacing between two strokes for this brush
     */
    double spacing() const;

    /**
     * @return the horizontal spacing
     */
    double xSpacing(double scale = 1.0) const;

    /**
     * @return the vertical spacing
     */
    double ySpacing(double scale = 1.0) const;

    /**
     * @return the width of the mask for the given scale
     */
    qint32 maskWidth(double scale, double angle) const;

    /**
     * @return the height of the mask for the given scale
     */
    qint32 maskHeight(double scale, double angle) const;

    virtual void setUseColorAsMask(bool useColorAsMask);

    virtual bool useColorAsMask() const;

    virtual bool hasColor() const;

    virtual void makeMaskImage();

    /**
     * @return the width (for scale == 1.0)
     */
    qint32 width() const;

    /**
     * @return the height (for scale == 1.0)
     */
    qint32 height() const;


    virtual enumBrushType brushType() const;

    virtual KisBoundary boundary();

    /**
     * Returns true if this brush can return something useful for the info. This is used
     * by Pipe Brushes that can't paint sometimes
     **/
    virtual bool canPaintFor(const KisPaintInformation& /*info*/);

    /**
     * Makes a copy of this brush.
     */
    virtual KisBrush* clone() const;

    /**
     * Serialize this brush to XML.
     */
    virtual void toXML(QDomDocument& , QDomElement&) const;

    /**
     * @return default file extension for saving the brush
     */
    virtual QString defaultFileExtension() const;

protected:
    void setWidth(qint32 w);
    void setHeight(qint32 h);
    void setImage(const QImage& img);
    void setBrushType(enumBrushType type);

private:

    bool init();
    bool initFromPaintDev(KisPaintDeviceSP image, int x, int y, int w, int h);
    void createScaledBrushes() const;

    KisQImagemaskSP scaleMask(const ScaledBrush *srcBrush, double scale, double subPixelX, double subPixelY) const;
    QImage scaleImage(const ScaledBrush *srcBrush, double scale, double subPixelX, double subPixelY) const;

    static QImage scaleImage(const QImage& srcImage, int width, int height);
    static QImage interpolate(const QImage& image1, const QImage& image2, double t);

    static KisQImagemaskSP scaleSinglePixelMask(double scale, quint8 maskValue, double subPixelX, double subPixelY);
    static QImage scaleSinglePixelImage(double scale, QRgb pixel, double subPixelX, double subPixelY);

    // Find the scaled brush(es) nearest to the given scale.
    void findScaledBrushes(double scale, const ScaledBrush **aboveBrush, const ScaledBrush **belowBrush) const;

    // Initialize our boundary
    void generateBoundary();

    struct Private;
    Private* const d;
};
#endif // KIS_BRUSH_

