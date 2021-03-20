/*
 * SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_PRESCALED_PROJECTION_H
#define KIS_PRESCALED_PROJECTION_H

#include <QObject>

#include <kritaui_export.h>
#include <kis_shared.h>

#include "KoColorConversionTransformation.h"
class QImage;
class QRect;
class QSize;
class QPainter;

class KoColorProfile;
class KisCoordinatesConverter;
class KisDisplayFilter;

#include <kis_types.h>
#include "kis_ui_types.h"



/**
 * KisPrescaledProjection is responsible for keeping around a
 * prescaled QImage representation that is always suitable for
 * painting onto the canvas.
 *
 * Note: the export macro is only for the unittest.
 */
class KRITAUI_EXPORT KisPrescaledProjection : public QObject, public KisShared
{
    Q_OBJECT
public:

    KisPrescaledProjection();
    ~KisPrescaledProjection() override;

    void setImage(KisImageWSP image);

    /**
     * Return the prescaled QImage. The prescaled image is exactly as big as
     * the canvas widget in pixels.
     */
    QImage prescaledQImage() const;

    void setCoordinatesConverter(KisCoordinatesConverter *coordinatesConverter);

public Q_SLOTS:

    /**
     * Retrieves image's data from KisImage object and updates
     * internal cache
     * @param dirtyImageRect the rect changed on the image
     * @see recalculateCache
     */
    KisUpdateInfoSP updateCache(const QRect &dirtyImageRect);

    /**
     * Updates the prescaled cache at current zoom level
     * @param info update structure returned by updateCache
     * @see updateCache
     */
    void recalculateCache(KisUpdateInfoSP info);

    /**
     * Called whenever the configuration settings change.
     */
    void updateSettings();

    /**
     * Called whenever the view widget needs to show a different part of
     * the document
     */
    void viewportMoved(const QPointF &offset);

    /**
     * Called whenever the size of the KisImage changes.
     * It is a part of a complex update ritual, when the size
     * fo the image changes. This method just resizes the storage
     * for the image cache, it doesn't update any cached data.
     */
    void slotImageSizeChanged(qint32 w, qint32 h);

    /**
     * Checks whether it is needed to resize the prescaled image and
     * updates it. The size is given in canvas widget pixels.
     */
    void notifyCanvasSizeChanged(const QSize &widgetSize);

    void notifyZoomChanged();

    /**
     * Set the current monitor profile
     */
    void setMonitorProfile(const KoColorProfile *monitorProfile, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags);

    void setChannelFlags(const QBitArray &channelFlags);

    void setDisplayFilter(QSharedPointer<KisDisplayFilter> displayFilter);

    /**
     * Called whenever the zoom level changes or another chunk of the
     * image becomes visible. The currently visible area of the image
     * is complete scaled again.
     */
    void preScale();

private:

    friend class KisPrescaledProjectionTest;

    KisPrescaledProjection(const KisPrescaledProjection &);
    KisPrescaledProjection operator=(const KisPrescaledProjection &);

    void updateViewportSize();

    /**
     * This creates an empty update information and fills it with the only
     * parameter: @p dirtyImageRect
     * This function is supposed to be run in the context of the image
     * threads, so it does no accesses to zoom or any UI specific values.
     * All the needed information for zooming will be fetched in the context
     * of the UI thread in fillInUpdateInformation().
     *
     * @see fillInUpdateInformation()
     */
    KisPPUpdateInfoSP getInitialUpdateInformation(const QRect &dirtyImageRect);

    /**
     * Prepare all the information about rects needed during
     * projection updating.
     *
     * @param viewportRect the part of the viewport that has to be updated
     * @param info the structure to be filled in. It's member dirtyImageRect
     * is supposed to have already been set up in the previous step of the
     * update in getInitialUpdateInformation(). Though it is allowed to
     * be null rect.
     *
     * @see getInitialUpdateInformation()
     */
    void fillInUpdateInformation(const QRect &viewportRect,
                                 KisPPUpdateInfoSP info);

    /**
     * Initiates the process of prescaled image update
     *
     * @param info prepared information
     */
    void updateScaledImage(KisPPUpdateInfoSP info);

    /**
     * Atual drawing is done here
     * @param info prepared information
     * @param gc The painter we draw on
     */
    void drawUsingBackend(QPainter &gc, KisPPUpdateInfoSP info);

    struct Private;
    Private * const m_d;
};

#endif
