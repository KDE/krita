/*
 *  Copyright (c) 2014 Manuel Riecke <spell1337@gmail.com>
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

#ifndef POSTERIZE_H
#define POSTERIZE_H

#include <QObject>
#include <QVariant>
#include "filter/kis_color_transformation_filter.h"
#include "kis_config_widget.h"

class Posterize : public QObject
{
    Q_OBJECT
public:
    Posterize(QObject *parent, const QVariantList &);
    ~Posterize() override;
};

class KisFilterPosterize : public KisColorTransformationFilter
{
public:
    KisFilterPosterize();
public:
    KoColorTransformation* createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const override;
    KisConfigWidget* createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
    static inline KoID id() {
        return KoID("posterize", i18n("Posterize"));
    }
protected:
    KisFilterConfigurationSP factoryConfiguration() const override;
};

class KisPosterizeColorTransformation : public KoColorTransformation
{
public:
    KisPosterizeColorTransformation(int steps, const KoColorSpace* cs);
    void transform(const quint8* src, quint8* dst, qint32 nPixels) const override;
private:
    const KoColorSpace* m_colorSpace;
    quint32 m_psize;
    quint16 m_step;
    quint16 m_halfStep;
};

#endif
