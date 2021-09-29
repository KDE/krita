/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisTag.h"

#include <QIODevice>
#include <QLocale>
#include <QBuffer>
#include <QByteArray>
#include <QStandardPaths>
#include <QFile>

#include <KLocalizedString>

#include "kconfigini_p.h"
#include "kconfigbackend_p.h"
#include "kconfigdata.h"

#include <kis_debug.h>

const QByteArray KisTag::s_group {"Desktop Entry"};
const QByteArray KisTag::s_type {"Type"};
const QByteArray KisTag::s_tag {"Tag"};
const QByteArray KisTag::s_name {"Name"};
const QByteArray KisTag::s_resourceType {"ResourceType"};
const QByteArray KisTag::s_url {"URL"};
const QByteArray KisTag::s_comment {"Comment"};
const QByteArray KisTag::s_defaultResources {"Default Resources"};

static QString currentLocale()
{
    const QStringList languages = KLocalizedString::languages();
    if (languages.isEmpty()) {
        return QLocale().name();
    } else {
        return languages.first();
    }
}

class KisTag::Private {
public:
    bool valid {false};
    QString url; // This is the actual tag
    QString name; // The translated tag name
    QString comment; // The translated tag comment
    QStringList defaultResources; // The list of resources as defined in the tag file
    QString resourceType; // The resource type this tag can be applied to
    KEntryMap map;
    QString filename; // the original filename for the tag
    int id {-1};
    bool active{true};
};

KisTag::KisTag()
    : d(new Private)
{
}

KisTag::~KisTag()
{
}

KisTag::KisTag(const KisTag &rhs)
    : d(new Private)
{
    *this = rhs;
}

KisTag &KisTag::operator=(const KisTag &rhs)
{
    if (this != &rhs) {
        d->valid = rhs.d->valid;
        d->url = rhs.d->url;
        d->name = rhs.d->name;
        d->resourceType = rhs.d->resourceType;
        d->comment = rhs.d->comment;
        d->defaultResources = rhs.d->defaultResources;
        d->map = rhs.d->map;
        d->filename = rhs.d->filename;
    }
    return *this;
}

KisTagSP KisTag::clone() const
{
    return KisTagSP(new KisTag(*this));
}

bool KisTag::valid() const
{
    return d->valid;
}

int KisTag::id() const
{
    return d->id;
}

bool KisTag::active() const
{
    return d->active;
}

QString KisTag::filename()
{
    return d->filename;
}

void KisTag::setFilename(const QString &filename)
{
    d->filename = filename;
}

QString KisTag::name() const
{
    return d->name;
}

void KisTag::setName(const QString &name)
{
    d->map.setEntry(s_group, s_name, name, KEntryMap::EntryDirty);
    d->name = name;
}

QString KisTag::url() const
{
    return d->url;
}

void KisTag::setUrl(const QString &url)
{
    d->map.setEntry(s_group, s_url, url, KEntryMap::EntryDirty);
    d->url = url;
}

QString KisTag::comment() const
{
    return d->comment;
}

void KisTag::setComment(const QString &comment)
{
    d->map.setEntry(s_group, s_comment, comment, KEntryMap::EntryDirty);
    d->comment = comment;
}

QString KisTag::resourceType() const
{
    return d->resourceType;
}

void KisTag::setResourceType(const QString &resourceType)
{
    d->resourceType = resourceType;
}

QStringList KisTag::defaultResources() const
{
    return d->defaultResources;
}

void KisTag::setDefaultResources(const QStringList &defaultResources)
{
    d->defaultResources = defaultResources;
}

bool KisTag::load(QIODevice &io)
{
    if (!io.isOpen()) {
        io.open(QIODevice::ReadOnly);
    }
    KIS_ASSERT(io.isOpen());

    KConfigIniBackend ini;
    KConfigBackend::ParseInfo r = ini.parseConfigIO(io, currentLocale().toUtf8(), d->map, KConfigBackend::ParseOption::ParseGlobal, false);
    if (r != KConfigBackend::ParseInfo::ParseOk) {
        qWarning() << "Could not load this tag file" << r;
        return false;
    }

    QString t = d->map.getEntry(s_group, s_type);
    if (t != s_tag) {
        qWarning() << "Not a tag desktop file" << t;
        return false;
    }

    d->url = d->map.getEntry(s_group, s_url);
    d->name = d->map.getEntry(s_group, s_name, QString(), KEntryMap::SearchLocalized);
    d->resourceType = d->map.getEntry(s_group, s_resourceType, QString(), KEntryMap::SearchLocalized);
    d->comment = d->map.getEntry(s_group, s_comment, QString(), KEntryMap::SearchLocalized);
    d->defaultResources = d->map.getEntry(s_group, s_defaultResources, QString()).split(',', QString::SkipEmptyParts);
    d->valid = true;

    return true;
}

bool KisTag::save(QIODevice &io)
{
    KConfigIniBackend ini;
    d->map.setEntry(s_group, s_type, s_tag, KEntryMap::EntryDirty);
    d->map.setEntry(s_group, s_url, d->url, KEntryMap::EntryDirty);
    d->map.setEntry(s_group, s_name, d->name, KEntryMap::EntryDirty);
    d->map.setEntry(s_group, s_resourceType, d->resourceType, KEntryMap::EntryDirty);
    d->map.setEntry(s_group, s_comment, d->comment, KEntryMap::EntryDirty);
    d->map.setEntry(s_group, s_defaultResources, d->defaultResources.join(','), KEntryMap::EntryDirty);

    ini.writeEntries(currentLocale().toUtf8(), io, d->map);
    return true;
}

void KisTag::setId(int id)
{
    d->id = id;
}

void KisTag::setActive(bool active)
{
    d->active = active;
}

void KisTag::setValid(bool valid)
{
    d->valid = valid;
}

