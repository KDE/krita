/*
 * Copyright (C) 2018 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KISTAGLOADER_H
#define KISTAGLOADER_H

#include <QDebug>
#include <QString>
#include <QScopedPointer>
#include <QSharedPointer>

class QIODevice;

#include "kritaresources_export.h"

class KisTag;
typedef QSharedPointer<KisTag> KisTagSP;


/**
 * @brief The KisTag loads a tag from a .tag file.
 * A .tag file is a .desktop file. The following fields
 * are important:
 *
 * name: the name of the tag, which can be translated
 * comment: a tooltip for the tag, which can be translagted
 * url: the untranslated name of the tag
 *
 */
class KRITARESOURCES_EXPORT KisTag
{
public:
    KisTag();
    virtual ~KisTag();
    KisTag(const KisTag &rhs);
    KisTag &operator=(const KisTag &rhs);
    KisTagSP clone() const;

    bool valid() const;

    int id() const;
    bool active() const;

    QString name() const;
    void setName(const QString &name);

    QString url() const;
    void setUrl(const QString &url);

    QString comment() const;
    void setComment(const QString &comment);

    QStringList defaultResources() const;
    void setDefaultResources(const QStringList &defaultResources);

    bool load(QIODevice &io);
    bool save(QIODevice &io);

    static bool compareNamesAndUrls(KisTagSP left, KisTagSP right);

private:

    friend class KisTagModel;
    friend class KisResourceModel;
    friend class KisTagChooserWidget;
    void setId(int id);
    void setActive(bool active);
    void setValid(bool valid);
    static const QByteArray s_group;
    static const QByteArray s_type;
    static const QByteArray s_tag;
    static const QByteArray s_name;
    static const QByteArray s_url;
    static const QByteArray s_comment;
    static const QByteArray s_defaultResources;
    class Private;
    QScopedPointer<Private> d;
};


inline QDebug operator<<(QDebug dbg, const KisTagSP tag)
{
    dbg.space() << "[TAG] Name" << tag->name()
                << "Url" << tag->url()
                << "Comment" << tag->comment()
                << "Default resources" << tag->defaultResources().join(", ");
    return dbg.space();
}

#endif // KISTAGLOADER_H
