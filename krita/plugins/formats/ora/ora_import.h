/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _IMPORT_H_
#define _IMPORT_H_

#include <KoFilter.h>

class OraImport : public KoFilter
{
    Q_OBJECT
public:
    OraImport(QObject* parent, const QStringList&);
    virtual ~OraImport();
public:
    virtual KoFilter::ConversionStatus convert(const QByteArray& from, const QByteArray& to);
};

#endif
