/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_CURVE_LABEL_H_
#define _KIS_CURVE_LABEL_H_

#include <krita_export.h>

class QString;
class QImage;

class PAINTOP_EXPORT KisCurveLabel
{
public:
    KisCurveLabel();
    KisCurveLabel(const QString& );
    KisCurveLabel(const QImage& );
    KisCurveLabel(const KisCurveLabel& );
    KisCurveLabel& operator=(const KisCurveLabel& );
    ~KisCurveLabel();

    QString name() const;
    QImage icon() const;
private:
    struct Private;
    Private* const d;
};


#endif
