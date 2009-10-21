/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_OPENEXR_IMPORT_H_
#define KIS_OPENEXR_IMPORT_H_

#include <KoFilter.h>
#include <half.h>

class KisOpenEXRImport : public KoFilter
{
    Q_OBJECT

public:
    KisOpenEXRImport(QObject* parent, const QStringList&);
    virtual ~KisOpenEXRImport();

public:
    virtual KoFilter::ConversionStatus convert(const QByteArray& from, const QByteArray& to);
    void setPixel(quint8 *dst, half red, half green, half blue, half alpha) const;
};

#endif // KIS_OPENEXR_IMPORT_H_

