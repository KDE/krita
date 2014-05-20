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

class IndexColors : public QObject
{
    Q_OBJECT
public:
    IndexColors(QObject *parent, const QVariantList &);
    virtual ~IndexColors();
};

class KisFilterIndexColors : public KisColorTransformationFilter
{
public:
    KisFilterIndexColors();
public:
    virtual KoColorTransformation* createTransformation(const KoColorSpace* cs, const KisFilterConfiguration* config) const;
    virtual KisConfigWidget* createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev) const;
    static inline KoID id() {
        return KoID("indexcolors", i18n("IndexColors"));
    }
protected:
    virtual KisFilterConfiguration* factoryConfiguration(const KisPaintDeviceSP) const;
};

struct LabColor
{
    quint16 L;
    quint16 a;
    quint16 b;
};

struct KisIndexColorPalette
{
    LabColor colors[256];
    int numColors;

    struct
    {
        float L;
        float a;
        float b;
    } similarityFactors;

    KisIndexColorPalette()
    {
        numColors = 0;
    };
    void insertShades(QColor clrA, QColor clrB, int shades);
    void insertShades(KoColor clrA, KoColor clrB, int shades);
    void insertShades(LabColor clrA, LabColor clrB, int shades);
    void insertColor(LabColor clr);
    void insertColor(KoColor clr);
    void insertColor(QColor clr);
    LabColor getNearestIndex(LabColor clr) const;
    float similarity(LabColor c0, LabColor c1) const;
};

class KisIndexColorTransformation : public KoColorTransformation
{
public:
    KisIndexColorTransformation(KisIndexColorPalette palette, const KoColorSpace* cs, int alphaSteps);
    virtual void transform(const quint8* src, quint8* dst, qint32 nPixels) const;
private:
    const KoColorSpace* m_colorSpace;
    quint32 m_psize;
    KisIndexColorPalette m_palette;
    quint16 m_alphaStep;
    quint16 m_alphaHalfStep;
};

#endif
