/* This file is part of the KDE project

   Copyright (C) 2010 KO GmbH <boud@kogmbh.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */
#ifndef KOODFLINENUMBERINGCONFIGURATION_H
#define KOODFLINENUMBERINGCONFIGURATION_H

#include <QString>
#include <QPair>
#include <QMetaType>

#include "KoXmlReader.h"
#include "KoXmlWriter.h"
#include "KoOdfNumberDefinition.h"
#include "koodf_export.h"

/**
 * Implement section 14.9.1: Line Numbering Configuration.
 *
 * A document can contain none or one line numbering configuration element
 * <text:linenumbering-configuration> within the <office:styles> element. If the
 * element is not present, a default line numbering configuration is used. The default line numbering
 * may vary on the office application software, but every document saved by an application that
 * supports line numbering should contain a line numbering configuration element.
 *
 * The attributes that may be associated with the <text:linenumbering-configuration>
 * element are:
 * <ul>
 * <li> Line numbering enable
 * <li> Number format
 * <li> Text style
 * <li> Increment
 * <li> Position
 * <li> Offset
 * <li> Count empty lines
 * <li> Count line in text boxes
 * <li> Restart numbering on every page
 * </ul>
 * The following element may be included in the <text:linenumbering-separator> element:
 * <li> Separator
 */
class KOODF_EXPORT KoOdfLineNumberingConfiguration
{
public:

    KoOdfLineNumberingConfiguration();
    ~KoOdfLineNumberingConfiguration();
    KoOdfLineNumberingConfiguration(const KoOdfLineNumberingConfiguration &other);
    KoOdfLineNumberingConfiguration &operator=(const KoOdfLineNumberingConfiguration &other);


    /**
     * load the line numbering configuration element
     */
    void loadOdf(const KoXmlElement &element);

    /**
     * save the line number configuration element
     */
    void saveOdf(KoXmlWriter * writer) const;

    /**
     * The text:number-lines attribute controls whether or not lines are numbered.
     */
    bool enabled() const;
    void setEnabled(bool enabled);

    /**
     * See section 12.2 for detailed information on number format attributes. The attributes
     * described in section 12.2 can also be associated with the <text:linenumbering-configuration>
     * element.
     */
    KoOdfNumberDefinition numberFormat() const;
    void setNumberFormat(const KoOdfNumberDefinition &numberFormat);

    /**
     * The text:style-name attribute specifies the text style for all line numbers. The value
     * of this attribute is the name of the text style that is applied to all line numbers.
     */
    QString textStyle() const;
    void setTextStyle(const QString &textStyle);

    /**
     * The text:increment attribute causes line numbers that are a multiple of the given increment to
     * be numbered. For example, if the increment is 5, only lines number 5, 10, 15, and so on are
     * numbered.
     */
    int increment() const;
    void setIncrement(int increment);

    /**
     * The text:position attribute determines whether the line numbers are printed on the left, right,
     * inner, or outer margins.
     */
    enum Position {
        Left,
        Right,
        Inner,
        Outer
    };

    Position position() const;
    void setPosition(Position position);

    /**
     * The text:offset attribute determines the distance between the line number and the margin.
     */
    int offset() const;
    void setOffset(int offset);

    /**
     * The text:count-empty-lines attribute determines whether or not empty lines are included in
     * the line count. If the value of this attribute is true, empty lines are included in the line count.
     */
    bool countEmptyLines() const;
    void setCountEmptyLines(bool countEmptyLines);

    /**
     * The text:count-in-text-boxes attribute determines whether or not text in text boxes is
     * included in the line count. If the value of this attribute is true, text within text boxes is included in
     * the line count.
     */
    bool countLinesInTextBoxes() const;
    void setCountLinesInTextBoxes(bool countLinesInTextBoxes);

    /**
     * The text:restart-on-page attribute determines whether or not the line count is reset to 1 at
     * the start of every page.
     *
     * If the value of this attribute is true, the line count is reset to 1 at the beginning of every page,
     * resulting in page -specific numbering of lines. The default value of this attribute is false,
     * resulting in document-specific numbering of lines.
     */
    bool restartNumberingOnEveryPage() const;
    void setRestartNumberingOnEveryPage(bool restartNumberingOnEveryPage);

    /**
     * The <text:linenumbering-separator> element contains the text that is displayed as a
     * separator. A separator is text that is displayed instead of a line number for lines where no number
     * is displayed.
     *
     * This element is contained in the line numbering configuration element. If the element is not
     * present, no separator is displayed.
     *
     * The element's text:increment attribute causes the separator to appear on lines that are a
     * multiple of the given increment. For example, if the increment is 2, only lines 2, 4, 6, and so on get
     * a separator, provided that no number is displayed already.
     */
    QString separator() const;
    void setSeparator(const QString &separator);

    int separatorIncrement() const;
    void setSeparatorIncrement(int separatorIncrement);

private:

    class Private;
    Private * const d;

};

Q_DECLARE_METATYPE(KoOdfLineNumberingConfiguration*)

#endif // KOODFLINENUMBERINGCONFIGURATION_H
