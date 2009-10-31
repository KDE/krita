/* This file is part of the KDE project
 * Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_koffice@gadz.org>
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

//KOffice includes
#include "kotext_export.h"
#include <KoDataCenter.h>
//#include "KoChangeTrackerElement.h"
class KoChangeTrackerElement;

#include <KoGenChange.h>
#include <KoGenChanges.h>

class KoXmlElement;

//KDE includes

//Qt includes
#include <QObject>
#include <QMetaType>
//#include <QTextCharFormat>
//#include <QTextFormat>
class QTextCursor;
class QTextFormat;
//#include <QHash>
class QString;
//#include <QList>
class QTextDocumentFragment;

class KOTEXT_EXPORT KoChangeTracker : public QObject, public KoDataCenter
{
    Q_OBJECT
public:
    KoChangeTracker(QObject *parent = 0);

    ~KoChangeTracker();

    void setEnabled(bool enabled);
    bool isEnabled();

    void setDisplayDeleted(bool enabled);
    bool displayDeleted();

    /// returns the changeId of the changeElement registered for the given change. This may be an already existing changeId, if the change could be merged.
    int getChangeId(QString &title, KoGenChange::Type type, QTextCursor &selection, QTextFormat &newFormat, int prevCharChangeId, int nextCharChangeId);

    int getFormatChangeId(QString title, QTextFormat &format, QTextFormat &prevFormat, int existingChangeId);
    int getInsertChangeId(QString title, int existingChangeId);
    int getDeleteChangeId(QString title, QTextDocumentFragment selection, int existingChangeId);

    KoChangeTrackerElement* elementById(int id);

    bool containsInlineChanges(const QTextFormat &format);
    int mergeableId(KoGenChange::Type type, QString &title, int existingId);

    /// Splits a changeElement. This creates a duplicate changeElement with a different changeId. This is used because we do not support overlapping change regions. The function returns the new changeId
    int split(int changeId);

    bool isParent(int testedId, int baseId);
    void setParent(int child, int parent);

    /// Load/save methods
    bool saveInlineChange(int changeId, KoGenChange &change);

    void loadOdfChanges(const KoXmlElement& element);
    int getLoadedChangeId(QString odfId);

private:

    /// reimplemented
    virtual bool completeLoading(KoStore *store);

    /// reimplemented
    virtual bool completeSaving(KoStore *store, KoXmlWriter *manifestWriter, KoShapeSavingContext *context);

    class Private;
    Private* const d;
};

Q_DECLARE_METATYPE(KoChangeTracker*)

#endif
