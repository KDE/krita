/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KOSVGTEXTSHAPEMARKUPCONVERTER_H
#define KOSVGTEXTSHAPEMARKUPCONVERTER_H

#include "kritaflake_export.h"

#include <QScopedPointer>
#include <QList>

class QRectF;
class KoSvgTextShape;

class KRITAFLAKE_EXPORT KoSvgTextShapeMarkupConverter
{
public:
    KoSvgTextShapeMarkupConverter(KoSvgTextShape *shape);
    ~KoSvgTextShapeMarkupConverter();

    bool convertToSvg(QString *svgText, QString *stylesText);
    bool convertFromSvg(const QString &svgText, const QString &stylesText, const QRectF &boundsInPixels, qreal pixelsPerInch);

    QStringList errors() const;
    QStringList warnings() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KOSVGTEXTSHAPEMARKUPCONVERTER_H
