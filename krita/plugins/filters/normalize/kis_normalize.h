/*
 * Copyright (c) 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
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

#ifndef NORMALIZE_H
#define NORMALIZE_H

#include <QObject>
#include <QVariant>
#include "filter/kis_color_transformation_filter.h"

class KritaNormalizeFilter : public QObject
{
    Q_OBJECT
public:
    KritaNormalizeFilter(QObject *parent, const QVariantList &);
    virtual ~KritaNormalizeFilter();
};

class KisFilterNormalize : public KisColorTransformationFilter
{
public:
    KisFilterNormalize();
public:
    virtual KoColorTransformation* createTransformation(const KoColorSpace* cs, const KisFilterConfiguration* config) const;
};

class KisNormalizeTransformation : public KoColorTransformation
{
public:
    KisNormalizeTransformation(const KoColorSpace* cs);
    virtual void transform(const quint8* src, quint8* dst, qint32 nPixels) const;
private:
    const KoColorSpace* m_colorSpace;
    quint32 m_psize;
};

#endif
