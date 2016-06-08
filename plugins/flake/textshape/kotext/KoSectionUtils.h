/*
 *  Copyright (c) 2014 Denis Kuplyakov <dener.kup@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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
#ifndef KOSECTIONUTILS_H
#define KOSECTIONUTILS_H

#include <KoSection.h>
#include <KoSectionEnd.h>

#include <QTextCursor>
#include <QVariant>
#include <QString>

namespace KoSectionUtils {
    /**
     * Moves the cursors to the next block within the same QTextFrame.
     * @param cur cursor to move, modified during call
     * @return @c false if there is no next block, @c true otherwise
     */
    bool getNextBlock(QTextCursor &cur);

    /**
     * Convinient function to set a list of startings to QTextBlockFormat.
     * This checks that list is empty.
     *
     * @param fmt QTextBlockFormat reference to set startings.
     * @param list QList<KoSection *> is a list to set.
     */
    KRITATEXT_EXPORT void setSectionStartings(QTextBlockFormat &fmt, const QList<KoSection *> &list);

    /**
     * Convinient function to set a list of endings to QTextBlockFormat.
     * This checks that list is empty.
     *
     * @param fmt QTextBlockFormat reference to set endings.
     * @param list QList<KoSectionEnd *> is a list to set.
     */
    KRITATEXT_EXPORT void setSectionEndings(QTextBlockFormat& fmt, const QList<KoSectionEnd *> &list);

    /**
     * Convinient function to get section startings from QTextBlockFormat.
     * @param fmt QTextBlockFormat format to retrieve section startings from.
     * @return QList<KoSection *> that contains pointers to sections that start
     *                            according to QTextBlockFormat.
     */
    KRITATEXT_EXPORT QList<KoSection *> sectionStartings(const QTextBlockFormat &fmt);

    /**
     * Convinient function to get section endings from QTextBlockFormat.
     * @param fmt QTextBlockFormat format to retrieve section startings from.
     * @return QList<KoSectionEnd *> that contains pointers to sections that end
     *                               according to QTextBlockFormat.
     */
    KRITATEXT_EXPORT QList<KoSectionEnd *> sectionEndings(const QTextBlockFormat& fmt);

}

#endif //KOSECTIONUTILS_H
