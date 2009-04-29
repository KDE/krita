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
#ifndef KOCHANGETRACKERELEMT_H
#define KOCHANGETRACKERELEMT_H

#include <QObject>
#include <QMetaType>
#include <QTextFormat>
#include <QString>

#include <KoGenChange.h>

#include "kotext_export.h"

class KOTEXT_EXPORT KoChangeTrackerElement
{
public:

    KoChangeTrackerElement(QString title, KoGenChange::Type type);

    KoChangeTrackerElement();

    ~KoChangeTrackerElement();

    void setEnabled(bool enabled);
    bool isEnabled();

    void setChangeType(KoGenChange::Type type);
    KoGenChange::Type getChangeType();

    void setChangeTitle(QString title);
    QString getChangeTitle();

    void setChangeFormat(QTextFormat &format);
    QTextFormat getChangeFormat();

    void setPrevFormat(QTextFormat &prevFormat);
    QTextFormat getPrevFormat();

    bool hasCreator();
    void setCreator(QString creator);
    QString getCreator();

    bool hasDate();
    void setDate(QString date);
    QString getDate();

    bool hasExtraMetaData();
    void setExtraMetaData(QString metaData);
    QString getExtraMetaData();

    bool hasDeleteData();
    void setDeleteData(QString data);
    QString getDeleteData();

private:
    class Private;
    Private* const d;
};

#endif
