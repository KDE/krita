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
#include <kis_dab_shape.h>
#include <kritabrush_export.h>

class KisQImagemask;
typedef KisSharedPtr<KisQImagemask> KisQImagemaskSP;

class QString;
class KoColor;
class KoColorSpace;

class KisPaintInformation;
class KisBoundary;
class KisPaintopLodLimitations;

enum enumBrushType {
    INVALID,
    MASK,
    IMAGE,
    PIPE_MASK,
    PIPE_IMAGE
};

static const qreal DEFAULT_SOFTNESS_FACTOR = 1.0;

class KisBrush;
typedef QSharedPointer<KisBrush> KisBrushSP;

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
class BRUSH_EXPORT KisBrush : public KoResource
{


public:
    class ColoringInformation
    {
    public:
        virtual ~ColoringInformation();
        virtual const quint8* color() const = 0;
        virtual void nextColumn() = 0;
        virtual void nextRow() = 0;
    };

protected:

    class PlainColoringInformation : public ColoringInformation
    {
    public:
        PlainColoringInformation(const quint8* color);
        ~PlainColoringInformation() override;
        const quint8* color() const override ;
        void nextColumn() override;
        void nextRow() override;
    private:
        const quint8* m_color;
    };

    class PaintDeviceColoringInformation : public ColoringInformation
    {

    public:

        PaintDeviceColoringInformation(const KisPaintDeviceSP source, int width);
        ~PaintDeviceColoringInformation() override;
        const quint8* color() const override ;
        void nextColumn() override;
        void nextRow() override;

    private:

        const KisPaintDeviceSP m_source;
        KisHLineConstIteratorSP m_iterator;
    };

public:

    KisBrush();
    KisBrush(const QString& filename);
    ~KisBrush() override;

    KisBrush(const KisBrush &rhs);
    KisBrush &operator=(const KisBrush &rhs);

    virtual qreal userEffectiveSize() const = 0;
    virtual void setUserEffectiveSize(qreal value) = 0;

    bool load() override {
        return false;
    }

    bool loadFromDevice(QIODevice *) override {
        return false;
    }


    bool save() override {
        return false;
    }

    bool saveToDevice(QIODevice* ) const override {
        return false;
    }

    /**
     * @brief brushImage the image the brush tip can paint with. Not all brush types have a single
     * image.
     * @return a valid QImage.
     */
    virtual QImage brushTipImage() const;

    /**
     * Change the spacing of the brush.
     * @param spacing a spacing of 1.0 means that strokes will be separated from one time the size
     *                of the brush.
     */
    virtual void setSpacing(double spacing);

    /**
     * @return the spacing between two strokes for this brush
     */
    double spacing() const;

    void setAutoSpacing(bool active, qreal coeff);

    bool autoSpacingActive() const;
    qreal autoSpacingCoeff() const;


    /**
     * @return the width (for scale == 1.0)
     */
    qint32 width() const;

    /**
     * @return the height (for scale == 1.0)
     */
    qint32 height() const;

    /**
     * @return the width of the mask for the given scale and angle
     */
    virtual qint32 maskWidth(KisDabShape const&, qreal subPixelX, qreal subPixelY, const KisPaintInformation& info) const;

    /**
     * @return the height of the mask for the given scale and angle
     */
    virtual qint32 maskHeight(KisDabShape const&, qreal subPixelX, qreal subPixelY, const KisPaintInformation& info) const;

    /**
     * @return the logical size of the brush, that is the size measured
     *         in floating point value.
     *
     *         This value should not be used for calculating future dab sizes
     *         because it doesn't take any rounding into account. The only use
     *         of this metric is calculation of brush-size derivatives like
     *         hotspots and spacing.
     */
     virtual QSizeF characteristicSize(KisDabShape const&) const;

    /**
     * @return the angle of the mask adding the given angle
     */
    double maskAngle(double angle = 0) const;

    /**
     * @return the index of the brush
     *         if the brush consists of multiple images
     */
    virtual quint32 brushIndex(const KisPaintInformation& info) const;

    /**
     * The brush type defines how the brush is used.
     */
    virtual enumBrushType brushType() const;

    QPointF hotSpot(KisDabShape const&, const KisPaintInformation& info) const;

    /**
     * Returns true if this brush can return something useful for the info. This is used
     * by Pipe Brushes that can't paint sometimes
     **/
    virtual bool canPaintFor(const KisPaintInformation& /*info*/);


    /**
     * Is called by the paint op when a paintop starts a stroke.  The
     * point is that we store brushes a server while the paint ops are
     * are recreated all the time. Is means that upon a stroke start
     * the brushes may need to clear its state.
     */
    virtual void notifyStrokeStarted();

    /**
     * Is called by the cache, when cache hit has happened.
     * Having got this notification the brush can update the counters
     * of dabs, generate some new random values if needed.
     *
     * * NOTE: one should use **either** notifyCachedDabPainted() or prepareForSeqNo()
     *
     * Currently, this is used by pipe'd brushes to implement
     * incremental and random parasites
     */
    virtual void notifyCachedDabPainted(const KisPaintInformation& info);

    /**
     * Is called by the multithreaded queue to prepare a specific brush
     * tip for the particular seqNo.
     *
     * NOTE: one should use **either** notifyCachedDabPainted() or prepareForSeqNo()
     *
     * Currently, this is used by pipe'd brushes to implement
     * incremental and random parasites
     */
    virtual void prepareForSeqNo(const KisPaintInformation& info, int seqNo);

    /**
     * Notify the brush if it can use QtConcurrent's threading capabilities in its
     * internal routines. By default it is allowed, but some paintops (who do their
     * own multithreading) may ask the brush to avoid internal threading.
     */
    void setThreadingAllowed(bool value);

    /**
     * \see setThreadingAllowed() for details
     */
    bool threadingAllowed() const;

    /**
     * Return a fixed paint device that contains a correctly scaled image dab.
     */
    virtual KisFixedPaintDeviceSP paintDevice(const KoColorSpace * colorSpace,
            KisDabShape const&,
            const KisPaintInformation& info,
            double subPixelX = 0, double subPixelY = 0) const;

    /**
     * clear dst fill it with a mask colored with KoColor
     */
    void mask(KisFixedPaintDeviceSP dst,
              const KoColor& color,
              KisDabShape const& shape,
              const KisPaintInformation& info,
              double subPixelX = 0, double subPixelY = 0, qreal softnessFactor = DEFAULT_SOFTNESS_FACTOR) const;

    /**
     * clear dst and fill it with a mask colored with the corresponding colors of src
     */
    void mask(KisFixedPaintDeviceSP dst,
              const KisPaintDeviceSP src,
              KisDabShape const& shape,
              const KisPaintInformation& info,
              double subPixelX = 0, double subPixelY = 0, qreal softnessFactor = DEFAULT_SOFTNESS_FACTOR) const;


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
     * @param shape a shape applied on the alpha mask
     * @param info the painting information (this is only and should only be used by
     *             KisImagePipeBrush and only to be backward compatible with the Gimp,
     *             KisImagePipeBrush is ignoring scale and angle information)
     * @param subPixelX sub position of the brush (contained between 0.0 and 1.0)
     * @param subPixelY sub position of the brush (contained between 0.0 and 1.0)
     * @param softnessFactor softness factor of the brush
     *
     * @return a mask computed from the grey-level values of the
     * pixels in the brush.
     */
    virtual void generateMaskAndApplyMaskOrCreateDab(KisFixedPaintDeviceSP dst,
            ColoringInformation* coloringInfo,
            KisDabShape const&,
            const KisPaintInformation& info,
            double subPixelX = 0, double subPixelY = 0, qreal softnessFactor = DEFAULT_SOFTNESS_FACTOR) const;


    /**
     * Serialize this brush to XML.
     */
    virtual void toXML(QDomDocument& , QDomElement&) const;

    static KisBrushSP fromXML(const QDomElement& element);

    virtual const KisBoundary* boundary() const;
    virtual QPainterPath outline() const;

    virtual void setScale(qreal _scale);
    qreal scale() const;
    virtual void setAngle(qreal _angle);
    qreal angle() const;

    void clearBrushPyramid();

    virtual void lodLimitations(KisPaintopLodLimitations *l) const;

protected:

    void setWidth(qint32 width);

    void setHeight(qint32 height);

    void setHotSpot(QPointF);

    /**
     * XXX
     */
    virtual void setBrushType(enumBrushType type);

    virtual void setHasColor(bool hasColor);

public:

    /**
     * The image is used to represent the brush in the gui, and may also, depending on the brush type
     * be used to define the actual brush instance.
     */
    virtual void setBrushTipImage(const QImage& image);

    /**
     * Returns true if the brush has a bunch of pixels almost
     * fully transparent in the very center. If the brush is pierced,
     * then dulling mode may not work correctly due to empty samples.
     *
     * WARNING: this method is relatively expensive since it iterates
     *          up to 100 pixels of the brush.
     */
    bool isPiercedApprox() const;

protected:

    void resetBoundary();

    void predefinedBrushToXML(const QString &type, QDomElement& e) const;

private:

    // Initialize our boundary
    void generateBoundary() const;

    struct Private;
    Private* const d;

};


#endif // KIS_BRUSH_

