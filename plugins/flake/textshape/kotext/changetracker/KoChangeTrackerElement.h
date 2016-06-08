/* This file is part of the KDE project
 * Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_calligra@gadz.org>
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
#ifndef KOCHANGETRACKERELEMENT_H
#define KOCHANGETRACKERELEMENT_H

#include <KoGenChange.h>

#include "kritatext_export.h"

class KUndo2MagicString;
class QString;
class QTextFormat;
class QTextDocumentFragment;


class KRITATEXT_EXPORT KoChangeTrackerElement
{
public:

    KoChangeTrackerElement(const KUndo2MagicString& title, KoGenChange::Type type);

    KoChangeTrackerElement();

    KoChangeTrackerElement(const KoChangeTrackerElement &other);

    ~KoChangeTrackerElement();

    void setEnabled(bool enabled);
    bool isEnabled() const;

    ///This flag is used when a change is accepted or rejected. When set, the change becomes transparent to functions like KoChangeTracker::isParent,... The KoChangeTrackerElement behaves like it has been destroyed. This is not done because of the undo/redo. A KoChangeTrackerElement can only be destroyed when its accept/reject command is destroyed.
    void setAcceptedRejected(bool set);
    bool acceptedRejected();

    void setValid(bool valid);
    bool isValid() const;

    void setChangeType(KoGenChange::Type type);
    KoGenChange::Type getChangeType() const;

    void setChangeTitle(const KUndo2MagicString& title);
    KUndo2MagicString getChangeTitle() const;

    void setChangeFormat(const QTextFormat &format);
    QTextFormat getChangeFormat() const;

    void setPrevFormat(const QTextFormat &prevFormat);
    QTextFormat getPrevFormat() const;

    bool hasCreator() const;
    void setCreator(const QString& creator);
    QString getCreator() const;

    bool hasDate() const;
    void setDate(const QString& date);
    QString getDate() const;

    bool hasExtraMetaData()const;
    void setExtraMetaData(const QString& metaData);
    QString getExtraMetaData() const;

    bool hasDeleteData() const;
    void setDeleteData(const QTextDocumentFragment& fragment);
    QTextDocumentFragment getDeleteData() const;


private:
    class Private;
    Private* const d;
};

#endif
