/*
 *  Copyright (c) 2014 Denis Kuplyakov <dener.kup@gmail.com>
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
#ifndef KOSECTIONUTILS_H
#define KOSECTIONUTILS_H

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
     * Convinient function to get name of a section from QVariant
     * that is really a KoSection *.
     *
     * There is no internal check that \p q is a KoSection *.
     *
     * @param q QVariant version of pointer to a KoSection
     * @return name of a specified section
     */
    QString sectionStartName(QVariant q);
    
    /**
     * Convinient function to get name of a section from QVariant
     * that is really a KoSectionEnd *.
     *
     * There is no internal check that \p q is a KoSectionEnd *.
     *
     * @param q QVariant version of pointer to a KoSectionEnd
     * @return name of a specified section
     */
    QString sectionEndName(QVariant q);
    
    /**
     * Convinient function to set a list of startings to text block format.
     * This checks that list is empty.
     * 
     * @param fmt QTextBlockFormat reference to set startings
     * @param list QList<QVariant> where QVariant is a link to KoSection.
     *             This is a list to set.
     */
    void setSectionStartings(QTextBlockFormat &fmt, const QList<QVariant> &list);
    
      /**
     * Convinient function to set a list of endings to text block format.
     * This checks that list is empty.
     * 
     * @param fmt QTextBlockFormat reference to set endings
     * @param list QList<QVariant> where QVariant is a link to KoSectionEnd.
     *             This is a list to set.
     */
    void setSectionEndings(QTextBlockFormat &fmt, const QList<QVariant> &list);
}

#endif //KOSECTIONUTILS_H
