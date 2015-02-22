/*
 *  Copyright (c) 2011 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KOSECTION_H
#define KOSECTION_H

#include "kotext_export.h"

#include <QString>
#include <QPair>
#include <QScopedPointer>
#include <QStandardItem>
#include <QTextCursor>

class KoXmlElement;
class KoShapeSavingContext;
class KoTextSharedLoadingData;
class KoSectionEnd;

class KoSectionPrivate;
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
    explicit KoSection(const QTextCursor &cursor);
    ~KoSection();

    /// Returns section name
    QString name() const;
    /// Returns starting and ending position of section in QTextDocument
    QPair<int, int> bounds() const;
    /// Returns section level. Root section has @c 0 level.
    int level() const;
    /** Tries to set section's name to @p name
     * @return @c false if there is a section with such name
     * and new name isn't accepted
     */
    bool setName(const QString &name);

    bool loadOdf(const KoXmlElement &element, KoTextSharedLoadingData *sharedData, bool stylesDotXml);
    void saveOdf(KoShapeSavingContext &context) const;

protected:
    const QScopedPointer<KoSectionPrivate> d_ptr;

private:
    Q_DISABLE_COPY(KoSection)
    Q_DECLARE_PRIVATE(KoSection)

    void setSectionEnd(KoSectionEnd *sectionEnd);
    void setBeginPos(int pos);
    void setEndPos(int pos);
    void setLevel(int level);
    void setModelItem(QStandardItem *item);
    QStandardItem *modelItem();

    friend class KoSectionManager;
    friend class KoSectionEnd;
};

#endif // KOSECTION_H
