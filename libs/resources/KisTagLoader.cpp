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

#include "KisTagLoader.h"

#include <QIODevice>
#include <QLocale>
#include <QBuffer>
#include <QByteArray>
#include <QStandardPaths>
#include <QFile>

#include "kconfigini_p.h"
#include "kconfigbackend_p.h"
#include "kconfigdata.h"

#include <kis_debug.h>

const QByteArray KisTagLoader::s_group {"Desktop Entry"};
const QByteArray KisTagLoader::s_type {"Type"};
const QByteArray KisTagLoader::s_tag {"Tag"};
const QByteArray KisTagLoader::s_name {"Name"};
const QByteArray KisTagLoader::s_url {"URL"};
const QByteArray KisTagLoader::s_comment {"Comment"};

class KisTagLoader::Private {
public:
    QString url;
    QString name;
    QString comment;
    KEntryMap map;


    QString expandString(const QString &value)
    {
        QString aValue = value;

        // check for environment variables and make necessary translations
        int nDollarPos = aValue.indexOf(QLatin1Char('$'));
        while (nDollarPos != -1 && nDollarPos + 1 < aValue.length()) {
            // there is at least one $
            if (aValue[nDollarPos + 1] == QLatin1Char('(')) {
                int nEndPos = nDollarPos + 1;
                // the next character is not $
                while ((nEndPos <= aValue.length()) && (aValue[nEndPos] != QLatin1Char(')'))) {
                    nEndPos++;
                }
                nEndPos++;
                QString cmd = aValue.mid(nDollarPos + 2, nEndPos - nDollarPos - 3);

                QString result;

    // FIXME: wince does not have pipes
    #ifndef _WIN32_WCE
                FILE *fs = popen(QFile::encodeName(cmd).data(), "r");
                if (fs) {
                    QTextStream ts(fs, QIODevice::ReadOnly);
                    result = ts.readAll().trimmed();
                    pclose(fs);
                }
    #endif
                aValue.replace(nDollarPos, nEndPos - nDollarPos, result);
                nDollarPos += result.length();
            } else if (aValue[nDollarPos + 1] != QLatin1Char('$')) {
                int nEndPos = nDollarPos + 1;
                // the next character is not $
                QStringRef aVarName;
                if (aValue[nEndPos] == QLatin1Char('{')) {
                    while ((nEndPos <= aValue.length()) && (aValue[nEndPos] != QLatin1Char('}'))) {
                        nEndPos++;
                    }
                    nEndPos++;
                    aVarName = aValue.midRef(nDollarPos + 2, nEndPos - nDollarPos - 3);
                } else {
                    while (nEndPos <= aValue.length() &&
                            (aValue[nEndPos].isNumber() ||
                             aValue[nEndPos].isLetter() ||
                             aValue[nEndPos] == QLatin1Char('_'))) {
                        nEndPos++;
                    }
                    aVarName = aValue.midRef(nDollarPos + 1, nEndPos - nDollarPos - 1);
                }
                QString env;
                if (!aVarName.isEmpty()) {
    #ifdef Q_OS_WIN
                    if (aVarName == QLatin1String("HOME")) {
                        env = QDir::homePath();
                    } else
    #endif
                    {
                        QByteArray pEnv = qgetenv(aVarName.toLatin1().constData());
                        if (!pEnv.isEmpty()) {
                            env = QString::fromLocal8Bit(pEnv.constData());
                        } else {
                            if (aVarName == QStringLiteral("QT_DATA_HOME")) {
                                env = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
                            } else if (aVarName == QStringLiteral("QT_CONFIG_HOME")) {
                                env = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
                            } else if (aVarName == QStringLiteral("QT_CACHE_HOME")) {
                                env = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation);
                            }
                        }
                    }
                    aValue.replace(nDollarPos, nEndPos - nDollarPos, env);
                    nDollarPos += env.length();
                } else {
                    aValue.remove(nDollarPos, nEndPos - nDollarPos);
                }
            } else {
                // remove one of the dollar signs
                aValue.remove(nDollarPos, 1);
                nDollarPos++;
            }
            nDollarPos = aValue.indexOf(QLatin1Char('$'), nDollarPos);
        }

        return aValue;
    }

};

KisTagLoader::KisTagLoader()
    : d(new Private)
{
}

KisTagLoader::~KisTagLoader()
{

}

QString KisTagLoader::name() const
{
    return d->name;
}

void KisTagLoader::setName(const QString &name) const
{
    d->map.setEntry(s_group, s_name, name, KEntryMap::EntryDirty);
    d->name = name;
}

QString KisTagLoader::url() const
{
    return d->url;
}

void KisTagLoader::setUrl(const QString &url) const
{
    d->map.setEntry(s_group, s_url, url, KEntryMap::EntryDirty);
    d->url = url;
}

QString KisTagLoader::comment() const
{
    return d->comment;
}

void KisTagLoader::setComment(const QString &comment) const
{
    d->map.setEntry(s_group, s_comment, comment, KEntryMap::EntryDirty);
    d->comment = comment;
}

bool KisTagLoader::load(QIODevice &io)
{
    if (!io.isOpen()) {
        io.open(QIODevice::ReadOnly);
    }
    KIS_ASSERT(io.isOpen());

    KConfigIniBackend ini;
    KConfigBackend::ParseInfo r = ini.parseConfigIO(io, QLocale().name().toUtf8(), d->map, KConfigBackend::ParseOption::ParseGlobal, false);
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
    d->comment = d->map.getEntry(s_group, s_comment, QString(), KEntryMap::SearchLocalized);

    return true;
}

bool KisTagLoader::save(QIODevice &io)
{
    KConfigIniBackend ini;
    ini.writeEntries(QLocale().name().toUtf8(), io, d->map);
    return true;
}

