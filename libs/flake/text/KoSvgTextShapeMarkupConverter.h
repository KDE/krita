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

/**
 * KoSvgTextShapeMarkupConverter is a utility class for converting a
 * KoSvgTextShape to/from user-editable markup/svg representation.
 *
 * Please note that the converted SVG is **not** the same as when saved into
 * .kra! Some attributes are dropped to make the editing is easier for the
 * user.
 */
class KRITAFLAKE_EXPORT KoSvgTextShapeMarkupConverter
{
public:
    KoSvgTextShapeMarkupConverter(KoSvgTextShape *shape);
    ~KoSvgTextShapeMarkupConverter();

    /**
     * Convert the text shape into two strings: text and styles. Styles string
     * is nonempty only when the text has some gradient/pattern attached. It is
     * intended to be places into a separate tab in the GUI.
     *
     * @return true on success
     */
    bool convertToSvg(QString *svgText, QString *stylesText);

    /**
     * @brief upload the svg representation of text into the shape
     * @param svgText <text> part of SVG
     * @param stylesText <defs> part of SVG (used only for gradients and patterns)
     * @param boundsInPixels bounds of the entire image in pixel. Used for parsing percentage units.
     * @param pixelsPerInch resolution of the image where we load the shape to
     *
     * @return true if the text was parsed successfully. Check `errors()` and `warnings()` for details.
     */
    bool convertFromSvg(const QString &svgText, const QString &stylesText, const QRectF &boundsInPixels, qreal pixelsPerInch);

    /**
     * A list of errors happened during loading the user's text
     */
    QStringList errors() const;

    /**
     * A list of warnings produced during loading the user's text
     */
    QStringList warnings() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KOSVGTEXTSHAPEMARKUPCONVERTER_H
