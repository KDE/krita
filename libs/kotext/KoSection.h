/*
 *  Copyright (c) 2011 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KOSECTION_H
#define KOSECTION_H

#include "kotext_export.h"

#include <QMetaType>
#include <QString>
#include <QTextCursor>

class KoXmlElement;
class KoShapeSavingContext;
class KoTextSharedLoadingData;

/**
 * Contains the information about the current text:section.
 *
 * The <text:section> element has the following attributes:
 *
 * <ul>
 * <li>text:condition
 * <li>text:display
 * <li>text:name
 * <li>text:protected
 * <li>text:protection-key
 * <li>text:protection-key-digest-algorithm
 * <li>text:style-name
 * <li>xml:id
 * </ul>
 * (odf spec v.12)
 */
class KOTEXT_EXPORT KoSection
{
public:

    KoSection();
    ~KoSection();
    KoSection(const KoSection& other);

    QString name() const;

    bool loadOdf(const KoXmlElement &element, KoTextSharedLoadingData *sharedData, bool stylesDotXml);
    void saveOdf(KoShapeSavingContext &context);
private:
    class Private;
    Private * const d;

};

/**
 * Marks the end of the given section
 */
struct KOTEXT_EXPORT KoSectionEnd {
    QString name; //< the name of the section we are closing

    void saveOdf(KoShapeSavingContext &context);
};

namespace KoSectionUtils {
    /**
     * Moves the cursors to the next block within the same QTextFrame
     * @param cur cursor to move, modified during call
     * @return @c false if there is no next block, @c true otherwise
     */
    bool getNextBlock(QTextCursor &cur);
    /**
     * Convinient function to get name of a section from QVariant
     * that is really a KoSection *
     *
     * There is no internal check that \p q is a KoSection *
     *
     * @param q QVariant version of pointer to a KoSection
     * @return name of a specified section
     */
    QString sectionStartName(QVariant q);
    /**
     * Convinient function to get name of a section from QVariant
     * that is really a KoSectionEnd *
     *
     * There is no internal check that \p q is a KoSectionEnd *
     *
     * @param q QVariant version of pointer to a KoSectionEnd
     * @return name of a specified section
     */
    QString sectionEndName(QVariant q);
}

Q_DECLARE_METATYPE(KoSection)

#endif // KOSECTION_H
