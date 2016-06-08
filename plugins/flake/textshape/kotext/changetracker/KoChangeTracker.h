/* This file is part of the KDE project
 * Copyright (C) 2008 Pierre Stirnweiss <pierre.stirnweiss_calligra@gadz.org>
 * Copyright (C) 2011 Boudewijn Rempt <boud@kogmbh.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KOCHANGETRACKER_H
#define KOCHANGETRACKER_H

#include "kritatext_export.h"
#include <KoXmlReaderForward.h>

#include <QObject>
#include <QMetaType>
#include <QVector>

#include <KoGenChange.h>

class KUndo2MagicString;

class KoGenChanges;
class KoChangeTrackerElement;
class KoFormatChangeInformation;

class QTextCursor;
class QTextFormat;
class QString;
class QTextDocumentFragment;
class QTextList;

class KRITATEXT_EXPORT KoChangeTracker : public QObject
{
    Q_OBJECT
public:

    enum ChangeSaveFormat {
        ODF_1_2 = 0,
        DELTAXML,
        UNKNOWN = 9999
    };

    explicit KoChangeTracker(QObject *parent = 0);
    ~KoChangeTracker();

    void setRecordChanges(bool enabled);
    bool recordChanges() const;

    void setDisplayChanges(bool enabled);
    bool displayChanges() const;

    /// XXX: these three are called "getXXX" but do change the state of the change tracker
    int getFormatChangeId(const KUndo2MagicString &title, const QTextFormat &format, const QTextFormat &prevFormat, int existingChangeId);
    int getInsertChangeId(const KUndo2MagicString &title, int existingChangeId);
    int getDeleteChangeId(const KUndo2MagicString &title, const QTextDocumentFragment &selection, int existingChangeId);

    void setFormatChangeInformation(int formatChangeId, KoFormatChangeInformation *formatInformation);
    KoFormatChangeInformation *formatChangeInformation(int formatChangeId) const;

    KoChangeTrackerElement* elementById(int id) const;
    bool removeById(int id, bool freeMemory = true);

    //Returns all the deleted changes
    int getDeletedChanges(QVector<KoChangeTrackerElement *>& deleteVector) const;

    bool containsInlineChanges(const QTextFormat &format) const;
    int mergeableId(KoGenChange::Type type, const KUndo2MagicString &title, int existingId) const;

    QColor getInsertionBgColor() const;
    QColor getDeletionBgColor() const;
    QColor getFormatChangeBgColor() const;

    void setInsertionBgColor(const QColor& bgColor);
    void setDeletionBgColor(const QColor& color);
    void setFormatChangeBgColor(const QColor& color);

    /// Splits a changeElement. This creates a duplicate changeElement with a different changeId. This is used because we do not support overlapping change regions. The function returns the new changeId
    int split(int changeId);

    bool isParent(int testedParentId, int testedChildId) const;
    void setParent(int child, int parent);
    int parent(int changeId) const;

    int createDuplicateChangeId(int existingChangeId);
    bool isDuplicateChangeId(int duplicateChangeId) const;
    int originalChangeId(int duplicateChangeId) const;

    void acceptRejectChange(int changeId, bool set);

    /// Load/save methods
    bool saveInlineChange(int changeId, KoGenChange &change);

    /**
     * @brief saveInlineChanges saves all the changes in the internal map, except
     * for the delete changes, which are changed independently using saveInlineChange.
     * @return an updated table of numerical, internal changeid's to xml:id strings.
     */
    QMap<int, QString> saveInlineChanges(QMap<int, QString> changeTransTable, KoGenChanges &genChanges);

    void loadOdfChanges(const KoXmlElement& element);
    int getLoadedChangeId(const QString &odfId) const;

    static QTextDocumentFragment generateDeleteFragment(const QTextCursor& cursor);
    static void insertDeleteFragment(QTextCursor &cursor);
    static int fragmentLength(const QTextDocumentFragment &fragment);

    QString authorName() const;
    void setAuthorName(const QString &authorName);

    ChangeSaveFormat saveFormat() const;
    void setSaveFormat(ChangeSaveFormat saveFormat);

private:

    static bool checkListDeletion(const QTextList &list, const QTextCursor &cursor);
    class Private;
    Private* const d;
};

Q_DECLARE_METATYPE(KoChangeTracker*)

#endif
