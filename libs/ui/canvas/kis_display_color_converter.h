/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_DISPLAY_COLOR_CONVERTER_H
#define __KIS_DISPLAY_COLOR_CONVERTER_H

#include <QScopedPointer>

#include <KoColorDisplayRendererInterface.h>
#include <KoColorConversionTransformation.h>

#include "kis_types.h"
#include "canvas/kis_display_filter.h"

class KoColor;
class KoColorProfile;
class KoCanvasResourceProvider;
class KoID;

/**
 * Special helper class that provides primitives for converting colors when
 * displaying. We have at least 3 color spaces:
 *
 * 1) Image color space (any: RGB, CMYK, Lab, etc)
 * 2) Display color space (a limited RGB color space)
 * 3) Color selectors color space (the one where color selectors generate
 *    their HSV-based colors. Right now it is sRGB.
 *
 * KoColor may be in any of these color spaces. QColor should always
 * be in the display color space only.
 */
class KRITAUI_EXPORT KisDisplayColorConverter : public QObject
{
    Q_OBJECT

public:
    KisDisplayColorConverter();
    KisDisplayColorConverter(KoCanvasResourceProvider *resourceManager, QObject *parent);
    ~KisDisplayColorConverter() override;

    void setImage(KisImageSP image);
    void setImageColorSpace(const KoColorSpace *cs);

    static KisDisplayColorConverter* dumbConverterInstance();

    KoColorDisplayRendererInterface* displayRendererInterface() const;

    const KoColorSpace* paintingColorSpace() const;
    void setMonitorProfile(const KoColorProfile *monitorProfile);
    void setDisplayFilter(QSharedPointer<KisDisplayFilter> displayFilter);

    QColor toQColor(const KoColor &c) const;
    KoColor approximateFromRenderedQColor(const QColor &c) const;

    bool canSkipDisplayConversion(const KoColorSpace *cs) const;
    KoColor applyDisplayFiltering(const KoColor &srcColor, const KoID &bitDepthId) const;
    void applyDisplayFilteringF32(KisFixedPaintDeviceSP device, const KoID &bitDepthId) const;


    /**
     * Converts the exactBounds() (!) of the \p srcDevice into QImage
     * properly rendered into display RGB space. Please note that the
     * offset of the image in QImage is always zero for efficiency
     * reasons.
     */
    QImage toQImage(KisPaintDeviceSP srcDevice) const;

    KoColor fromHsv(int h, int s, int v, int a = 255) const;
    KoColor fromHsvF(qreal h, qreal s, qreal v, qreal a = 1.0);
    KoColor fromHslF(qreal h, qreal s, qreal l, qreal a = 1.0);
    KoColor fromHsiF(qreal h, qreal s, qreal i);
    KoColor fromHsyF(qreal h, qreal s, qreal y, qreal R=0.2126, qreal G=0.7152, qreal B=0.0722, qreal gamma=2.2);

    void getHsv(const KoColor &srcColor, int *h, int *s, int *v, int *a = 0) const;
    void getHsvF(const KoColor &srcColor, qreal *h, qreal *s, qreal *v, qreal *a = 0);
    void getHslF(const KoColor &srcColor, qreal *h, qreal *s, qreal *l, qreal *a = 0);
    void getHsiF(const KoColor &srcColor, qreal *h, qreal *s, qreal *i);
    void getHsyF(const KoColor &srcColor, qreal *h, qreal *s, qreal *y, qreal R=0.2126, qreal G=0.7152, qreal B=0.0722, qreal gamma=2.2);

    static KoColorConversionTransformation::Intent renderingIntent();
    static KoColorConversionTransformation::ConversionFlags conversionFlags();

    QSharedPointer<KisDisplayFilter> displayFilter() const;
    const KoColorProfile* monitorProfile() const;
    const KoColorProfile* openGLCanvasSurfaceProfile() const;
    bool isHDRMode() const;

    void notifyOpenGLCanvasIsActive(bool value);

Q_SIGNALS:
    void displayConfigurationChanged();

private:
    // is not possible to implement!
    KoColor toKoColor(const QColor &c);
    template <class Policy>
    typename Policy::Result convertToDisplayImpl(const KoColor &srcColor, bool alreadyInDestinationF32 = false) const;

private:
    Q_PRIVATE_SLOT(m_d, void slotCanvasResourceChanged(int key, const QVariant &v));
    Q_PRIVATE_SLOT(m_d, void selectPaintingColorSpace());
    Q_PRIVATE_SLOT(m_d, void slotUpdateCurrentNodeColorSpace());

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_DISPLAY_COLOR_CONVERTER_H */
