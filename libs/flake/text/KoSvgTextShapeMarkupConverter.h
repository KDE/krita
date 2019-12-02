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
#include <QTextDocument>
#include <QTextCharFormat>

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
     * is non-empty only when the text has some gradient/pattern attached. It is
     * intended to be places into a separate tab in the GUI.
     *
     * @return true on success
     */
    bool convertToSvg(QString *svgText, QString *stylesText);

    /**
     * @brief upload the svg representation of text into the shape
     * @param svgText \<text\> part of SVG
     * @param stylesText \<defs\> part of SVG (used only for gradients and patterns)
     * @param boundsInPixels bounds of the entire image in pixel. Used for parsing percentage units.
     * @param pixelsPerInch resolution of the image where we load the shape to
     *
     * @return true if the text was parsed successfully. Check `errors()` and `warnings()` for details.
     */
    bool convertFromSvg(const QString &svgText, const QString &stylesText, const QRectF &boundsInPixels, qreal pixelsPerInch);

    /**
     * @brief convertToHtml convert the text in the text shape to html
     * @param htmlText will be filled with correct html representing the text in the shape
     * @return @c true on success
     */
    bool convertToHtml(QString *htmlText);

    /**
     * @brief convertFromHtml converted Qt rich text html (and no other: https://doc.qt.io/qt-5/richtext-html-subset.html) to SVG
     * @param htmlText the input html
     * @param svgText the converted svg text element
     * @param styles
     * @return @c true if the conversion was successful
     */
    bool convertFromHtml(const QString &htmlText, QString *svgText, QString *styles);

    /**
     * @brief convertDocumentToSvg
     * @param doc the QTextDocument to convert.
     * @param svgText the converted svg text element
     * @return @c true if the conversion was successful
     */
    bool convertDocumentToSvg(const QTextDocument *doc, QString *svgText);

    /**
     * @brief convertSvgToDocument
     * @param svgText the \<text\> element and it's children as a string.
     * @param doc the QTextDocument that the conversion is written to.
     * @return @c true if the conversion was successful
     */
    bool convertSvgToDocument(const QString &svgText, QTextDocument *doc);


    /**
     * A list of errors happened during loading the user's text
     */
    QStringList errors() const;

    /**
     * A list of warnings produced during loading the user's text
     */
    QStringList warnings() const;

    /**
     * @brief style
     * creates a style string based on the blockformat and the format.
     * @param format the textCharFormat of the current text.
     * @param blockFormat the block format of the current text.
     * @param mostCommon the most common format to compare the format to.
     * @return a string that can be written into a style element.
     */
    QString style(QTextCharFormat format, QTextBlockFormat blockFormat, QTextCharFormat mostCommon = QTextCharFormat());

    /**
     * @brief stylesFromString
     * returns a qvector with two textformats:
     * at 0 is the QTextCharFormat
     * at 1 is the QTextBlockFormat
     * @param styles a style string split at ";"
     * @param currentCharFormat the current charformat to compare against.
     * @param currentBlockFormat the current blockformat to compare against.
     * @return A QVector with at 0 a QTextCharFormat and at 1 a QBlockCharFormat.
     */
    static QVector<QTextFormat> stylesFromString(QStringList styles, QTextCharFormat currentCharFormat, QTextBlockFormat currentBlockFormat);
    /**
     * @brief formatDifference
     * A class to get the difference between two text-char formats.
     * @param test the format to test
     * @param reference the format to test against.
     * @return the difference between the two.
     */
    QTextFormat formatDifference(QTextFormat test, QTextFormat reference);

private:
    struct Private;
    const QScopedPointer<Private> d;
};

#endif // KOSVGTEXTSHAPEMARKUPCONVERTER_H
