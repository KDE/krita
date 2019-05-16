/*
 * Copyright 2014 Manuel Riecke <spell1337@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The author disclaim all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.  In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 */

#ifndef INDEXCOLORS_H
#define INDEXCOLORS_H

#include <QObject>
#include <QVariant>
#include "filter/kis_color_transformation_filter.h"
#include "kis_config_widget.h"
#include <KoColor.h>

#include "indexcolorpalette.h"

class IndexColors : public QObject
{
    Q_OBJECT
public:
    IndexColors(QObject *parent, const QVariantList &);
    ~IndexColors() override;
};

class KisFilterIndexColors : public KisColorTransformationFilter
{
public:
    KisFilterIndexColors();
public:
    KoColorTransformation* createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const override;
    KisConfigWidget* createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
    static inline KoID id() {
        return KoID("indexcolors", i18n("Index Colors"));
    }
protected:
    KisFilterConfigurationSP factoryConfiguration() const override;
};

class KisIndexColorTransformation : public KoColorTransformation
{
public:
    KisIndexColorTransformation(IndexColorPalette palette, const KoColorSpace* cs, int alphaSteps);
    void transform(const quint8* src, quint8* dst, qint32 nPixels) const override;
private:
    const KoColorSpace* m_colorSpace;
    quint32 m_psize;
    IndexColorPalette m_palette;
    quint16 m_alphaStep;
    quint16 m_alphaHalfStep;
};

#endif
