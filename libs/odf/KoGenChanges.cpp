/* This file is part of the KDE project
  Copyright (C) 2008 Pierre Stirnweiss <pierre.stirnweiss_koffice@gadz.org>
   Copyright (C) 2010 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoGenChanges.h"
#include <KoXmlWriter.h>

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QMultiMap>
#include <QtCore/QSet>
#include <QtCore/QString>

#include <kdebug.h>

class KoGenChanges::Private
{
public:
    Private(KoGenChanges *q) : q(q) { }

    QString makeUniqueName(const QString &base) const;

    struct NamedChange {
        const KoGenChange* change; ///< @note owned by the collection
        QString name;
    };

    /// style definition -> name
    QMap<KoGenChange, QString>  changeMap;

    /// Map with the change name as key.
    /// This map is mainly used to check for name uniqueness
    QSet<QString> changeNames;

    /// List of styles (used to preserve ordering)
    QList<NamedChange> changeArray;
    QMap<KoGenChange, QString> ::iterator insertChange(const KoGenChange &change, const QString &name);

    KoGenChanges *q;
};

KoGenChanges::KoGenChanges()
    : d(new Private(this))
{
}

KoGenChanges::~KoGenChanges()
{
    delete d;
}

QString KoGenChanges::insert(const KoGenChange& change, const QString& name)
{
    QMap<KoGenChange, QString> ::iterator it = d->changeMap.find(change);
    if (it == d->changeMap.end()) {
        it = d->insertChange(change, name);
    }
    return it.value();
}

QMap<KoGenChange, QString>::iterator KoGenChanges::Private::insertChange(const KoGenChange &change, const QString &name)
{
    QString changeName(name);
    if (changeName.isEmpty()) {
        switch (change.type()) {
        case KoGenChange::InsertChange: changeName = 'I'; break;
        case KoGenChange::FormatChange: changeName = 'F'; break;
        case KoGenChange::DeleteChange: changeName = 'D'; break;
        default:
            changeName = 'C';
        }
    }
    changeName = makeUniqueName(changeName);
    changeNames.insert(changeName);
    QMap<KoGenChange, QString>::iterator it = changeMap.insert(change, changeName);
    NamedChange s;
    s.change = &it.key();
    s.name = changeName;
    changeArray.append(s);

    return it;
}

QMap<KoGenChange, QString> KoGenChanges::changes() const
{
    return d->changeMap;
}

QString KoGenChanges::Private::makeUniqueName(const QString& base) const
{
    if (!changeNames.contains(base))
        return base;
    int num = 1;
    QString name;
    do {
        name = base;
        name += QString::number(num++);
    } while (changeNames.contains(name));
    return name;
}

const KoGenChange* KoGenChanges::change(const QString& name) const
{
    QList<KoGenChanges::Private::NamedChange>::const_iterator it = d->changeArray.constBegin();
    const QList<KoGenChanges::Private::NamedChange>::const_iterator end = d->changeArray.constEnd();
    for (; it != end ; ++it) {
        if ((*it).name == name)
            return (*it).change;
    }
    return 0;
}

void KoGenChanges::saveOdfChanges(KoXmlWriter* xmlWriter) const
{
    xmlWriter->startElement("text:tracked-changes");

    QMap<KoGenChange, QString> changesList = changes();
    QMap<KoGenChange, QString>::const_iterator it = changesList.constBegin();
    for (; it != changesList.constEnd() ; ++it) {
        it.key().writeChange(xmlWriter, it.value());
    }

    xmlWriter->endElement(); // text:tracked-changes
}
