/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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

    QString filename();
    void setFilename(const QString &fileName);

    /// The unique identifier for the tag. Since tag urls are compared COLLATE NOCASE, tag urls must be ASCII only.
    QString url() const;
    void setUrl(const QString &url);

    /// The translated name of the tag
    QString name() const;
    void setName(const QString &name);

    /// a translated tooltip for the tag
    QString comment() const;
    void setComment(const QString &comment);

    QString resourceType() const;
    void setResourceType(const QString &resourceType);

    QStringList defaultResources() const;
    void setDefaultResources(const QStringList &defaultResources);

    bool load(QIODevice &io);
    bool save(QIODevice &io);

private:

    friend class KisTagModel;
    friend class KisAllTagsModel;
    friend class KisAllTagResourceModel;
    friend class KisAllResourcesModel;
    friend class KisResourceModel;
    friend class KisTagChooserWidget;
    friend class TestTagModel;
    friend class KisResourceLocator;

    void setId(int id);
    void setActive(bool active);
    void setValid(bool valid);

    static const QByteArray s_group;
    static const QByteArray s_type;
    static const QByteArray s_tag;
    static const QByteArray s_name;
    static const QByteArray s_resourceType;
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

Q_DECLARE_METATYPE(QSharedPointer<KisTag>)

#endif // KISTAGLOADER_H
