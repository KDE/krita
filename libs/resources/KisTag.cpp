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
#include <QTextCodec>
#include <QTextStream>

#include <KLocalizedString>

#include <kis_debug.h>

const QString KisTag::s_group {"Desktop Entry"};
const QString KisTag::s_type {"Type"};
const QString KisTag::s_tag {"Tag"};
const QString KisTag::s_name {"Name"};
const QString KisTag::s_resourceType {"ResourceType"};
const QString KisTag::s_url {"URL"};
const QString KisTag::s_comment {"Comment"};
const QString KisTag::s_defaultResources {"Default Resources"};
const QString KisTag::s_desktop {"[Desktop Entry]"};

class KisTag::Private {
public:
    bool valid {false};
    QString url; // This is the actual tag
    QString name;
    QString comment;
    QMap<QString, QString> names; // The translated tag names
    QMap<QString, QString> comments; // The translated tag comments
    QStringList defaultResources; // The list of resources as defined in the tag file
    QString resourceType; // The resource type this tag can be applied to
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
        d->comment = rhs.d->comment;
        d->names = rhs.d->names;
        d->comments = rhs.d->comments;
        d->defaultResources = rhs.d->defaultResources;        d->resourceType = rhs.d->resourceType;
        d->filename = rhs.d->filename;
        d->id = rhs.d->id;
        d->active = rhs.d->active;
    }
    return *this;
}

KisTagSP KisTag::clone() const
{
    return KisTagSP(new KisTag(*this));
}

QString KisTag::currentLocale()
{
    const QStringList languages = KLocalizedString::languages();
    QString locale;
    if (languages.isEmpty()) {
        locale = QLocale().name();
    }
    else {
        locale = languages.first();
    }
    return locale;
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

QString KisTag::name(bool translated) const
{
    if (translated && d->names.contains(currentLocale())) {
        return d->names[currentLocale()];
    }
    Q_ASSERT(!d->name.isEmpty());
    return d->name;
}

void KisTag::setName(const QString &name)
{
    d->name = name;
}

QMap<QString, QString> KisTag::names() const
{
    return d->names;
}

void KisTag::setNames(const QMap<QString, QString> &names)
{
    d->names = names;
}

QString KisTag::comment(bool translated) const
{
    if (translated && d->comments.contains(currentLocale())) {
        return d->comments[currentLocale()];
    }
    return d->comment;
}

void KisTag::setComment(const QString comment)
{
    d->comment = comment;
}

QString KisTag::url() const
{
    return d->url;
}

void KisTag::setUrl(const QString &url)
{
    d->url = url;
}


QMap<QString, QString> KisTag::comments() const
{
    return d->comments;
}

void KisTag::setComments(const QMap<QString, QString> &comments)
{
    d->comments = comments;
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

    const QList<QByteArray> lines = io.readAll().split('\n');
    if (lines.length() < 6 ) {
        qWarning()  << d->filename << ": Incomplete tag file" << lines.length();
        return false;
    }
    if (lines[0].toUpper() != s_desktop.toUpper()) {
        qWarning()  << d->filename << ":Invalid tag file" << lines[0];
        return false;
    }

    for (int i = 1; i < lines.length(); ++i) {

        QString line = QString::fromUtf8(lines[i]);

        if (line.isEmpty()) {
            continue;
        }

        if (!line.contains("=")) {
            qWarning() << "Found invalid line:" << line;
            continue;
        }
        int isPos = line.indexOf("=");
        QString key = line.left(isPos).trimmed();
        QString value = line.right(line.size() - (isPos + 1)).trimmed();

        if (key == s_url) {
            d->url = value;
        }
        else if (key == s_resourceType) {
            d->resourceType = value;
        }
        else if (key == s_defaultResources) {
            d->defaultResources = value.split(',', QString::SkipEmptyParts);
        }
        else if (key == s_name) {
            d->name = value;
        }
        else if (key == s_comment) {
            d->comment = value;
        }
        else if (key.startsWith(s_name + "[")) {
            int start = key.indexOf('[') + 1;
            int len = key.size() - (s_name.size() + 2);
            QString language = key.mid(start, len);
            d->names[language] = value;
        }
        else if (key.startsWith(s_comment + "[")) {
            int start = key.indexOf('[') + 1;
            int len = key.size() - (s_comment.size() + 2);
            QString language = key.mid(start, len);
            d->comments[language] = value;
        }
    }

    return true;
}

bool KisTag::save(QIODevice &io)
{
    QTextStream stream(&io);
    stream << s_desktop << '\n';
    stream << s_type << '=' << s_tag << '\n';
    stream << s_url << '=' << d->url << '\n';
    stream << s_resourceType << '=' << d->resourceType << '\n';
    stream << s_name << '=' << d->name << '\n';
    stream << s_comment << '=' << d->comment << '\n';
    stream << s_defaultResources << '=' << d->defaultResources.join(',') << '\n';

    Q_FOREACH(const QString &language, d->names) {
        stream << s_name << '[' << language << "]=" << d->names[language] << '\n';
    }

    Q_FOREACH(const QString &language, d->comments) {
        stream << s_comment << '[' << language << "]=" << d->comments[language] << '\n';
    }

    return false;
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

