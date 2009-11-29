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

#include <KoResource.h>

#include <kis_types.h>
#include <kis_shared.h>
#include <kis_paint_information.h>
#include <kis_fixed_paint_device.h>
#include <krita_export.h>

class KisQImagemask;
typedef KisSharedPtr<KisQImagemask> KisQImagemaskSP;

class QString;
class QPoint;
class QIODevice;

class KoColor;
class KoColorSpace;

class KisBoundary;
class KisQImageSP;
class KisScaledBrush;

enum enumBrushType {
    INVALID,
    MASK,
    IMAGE,
    PIPE_MASK,
    PIPE_IMAGE,
    AIRBRUSH
};



class KisBrush;
typedef KisSharedPtr<KisBrush> KisBrushSP;

/**
 * KisBrush is the base class for brush resources. A brush resource
 * defines one or more images that are used to potato-stamp along
 * the drawn path. The brush type defines how this brush is used --
 * the important difference is between masks (which take the current
 * painting color) and images (which do not). It is up to the paintop
 * to make use of this feature.
 *
 * Brushes must be serializable to an xml representation and provide
 * a factory class that can recreate or retrieve the brush based on
 * this representation.
 *
 * XXX: This api is still a big mess -- it needs a good refactoring.
 * And the whole KoResource architecture is way over-designed.
 */
class BRUSH_EXPORT KisBrush : public KoResource, public KisShared
{

    class ScaledBrush;

protected:

    class ColoringInformation
    {
    public:
        virtual ~ColoringInformation();
        virtual const quint8* color() const = 0;
        virtual void nextColumn() = 0;
        virtual void nextRow() = 0;
    };

    class PlainColoringInformation : public ColoringInformation
    {
    public:
        PlainColoringInformation(const quint8* color);
        virtual ~PlainColoringInformation();
        virtual const quint8* color() const ;
        virtual void nextColumn();
        virtual void nextRow();
    private:
        const quint8* m_color;
    };

    class PaintDeviceColoringInformation : public ColoringInformation
    {

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

    KisBrush();
    KisBrush(const QString& filename);

    virtual ~KisBrush();

    virtual bool load() {
        return false;
    }
    virtual bool save() {
        return false;
    }

    /**
     * @return the horizontal spacing
     */
    double xSpacing(double scale = 1.0) const;

    /**
     * @return the vertical spacing
     */
    double ySpacing(double scale = 1.0) const;

    /**
     * @return a preview of the brush
     */
    virtual QImage image() const;

    /**
     * Change the spacing of the brush.
     * @param spacing a spacing of 1.0 means that strokes will be separated from one time the size
     *                of the brush.
     */
    void setSpacing(double spacing);

    /**
     * @return the spacing between two strokes for this brush
     */
    double spacing() const;

    /**
     * @return the width (for scale == 1.0)
     */
    qint32 width() const;

    /**
     * @return the height (for scale == 1.0)
     */
    qint32 height() const;

    /**
     * @return the width of the mask for the given scale
     */
    qint32 maskWidth(double scale, double angle) const;

    /**
     * @return the height of the mask for the given scale
     */
    qint32 maskHeight(double scale, double angle) const;

    /**
     * The brush type defines how the brush is used.
     */
    virtual enumBrushType brushType() const;

    virtual QPointF hotSpot(double scaleX, double scaleY, double rotation = 0.0) const;

    /**
     * Returns true if this brush can return something useful for the info. This is used
     * by Pipe Brushes that can't paint sometimes
     **/
    virtual bool canPaintFor(const KisPaintInformation& /*info*/);

    /**
     * Return a fixed paint device that contains a correctly scaled image dab.
     */
    virtual KisFixedPaintDeviceSP paintDevice(const KoColorSpace * colorSpace,
                                              double scale, double angle,
                                              const KisPaintInformation& info,
                                              double subPixelX = 0, double subPixelY = 0) const;

    /**
     * Apply the brush mask to the pixels in dst. Dst should be big enough!
     */
    void mask(KisFixedPaintDeviceSP dst,
              double scaleX, double scaleY, double angle,
              const KisPaintInformation& info = KisPaintInformation(),
              double subPixelX = 0, double subPixelY = 0) const;

    /**
     * clear dst fill it with a mask colored with KoColor
     */
    void mask(KisFixedPaintDeviceSP dst,
              const KoColor& color,
              double scaleX, double scaleY, double angle,
              const KisPaintInformation& info = KisPaintInformation(),
              double subPixelX = 0, double subPixelY = 0) const;

    /**
     * clear dst and fill it with a mask colored with the corresponding colors of src
     */
    void mask(KisFixedPaintDeviceSP dst,
              const KisPaintDeviceSP src,
              double scaleX, double scaleY, double angle,
              const KisPaintInformation& info = KisPaintInformation(),
              double subPixelX = 0, double subPixelY = 0) const;


    virtual bool hasColor() const;

    /**
     * Create a mask and either mask dst (that is, change all alpha values of the
     * existing pixels to those of the mask) or, if coloringInfo is present, clear
     * dst and fill dst with pixels according to coloringInfo, masked according to the
     * generated mask.
     *
     * @param dst the destination that will be draw on the image, and this function
     *            will edit its alpha channel
     * @param coloringInfo coloring information that will be copied on the dab, it can be null
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
    virtual void generateMaskAndApplyMaskOrCreateDab(KisFixedPaintDeviceSP dst,
                                                     ColoringInformation* coloringInfo,
                                                     double scaleX, double scaleY, double angle,
                                                     const KisPaintInformation& info = KisPaintInformation(),
                                                     double subPixelX = 0, double subPixelY = 0) const;


    /**
     * Serialize this brush to XML.
     */
    virtual void toXML(QDomDocument& , QDomElement&) const {};

    static KisBrushSP fromXML(const QDomElement& element);

    virtual const KisBoundary* boundary() const;

protected:

    KisBrush(const KisBrush& rhs);

    void setWidth(qint32 width);

    void setHeight(qint32 height);

    void setHotSpot(QPointF);

    /**
     * The image is used to represent the brush in the gui, and may also, depending on the brush type
     * be used to define the actual brush instance.
     */
    virtual void setImage(const QImage& image);

    /**
     * XXX
     */
    void setBrushType(enumBrushType type);

    void clearScaledBrushes();

    void createScaledBrushes() const;

    void setHasColor(bool hasColor);

protected:

    QImage m_image;

    void resetBoundary();

private:

    KisQImagemaskSP createMask(double scale, double subPixelX, double subPixelY) const;

    KisQImagemaskSP scaleMask(const KisScaledBrush *srcBrush,
                              double scale, double subPixelX, double subPixelY) const;

    QImage scaleImage(const KisScaledBrush *srcBrush,
                      double scale, double subPixelX, double subPixelY) const;

    static QImage scaleImage(const QImage& srcImage, int width, int height);

    static QImage interpolate(const QImage& image1, const QImage& image2, double t);

    static KisQImagemaskSP scaleSinglePixelMask(double scale, quint8 maskValue,
                                                double subPixelX, double subPixelY);

    static QImage scaleSinglePixelImage(double scale, QRgb pixel,
                                        double subPixelX, double subPixelY);

    // Find the scaled brush(es) nearest to the given scale.
    void findScaledBrushes(double scale,
                           const KisScaledBrush **aboveBrush,
                           const KisScaledBrush **belowBrush) const;

    // Initialize our boundary
    void generateBoundary() const;

    struct Private;
    Private* const d;

};


#endif // KIS_BRUSH_

