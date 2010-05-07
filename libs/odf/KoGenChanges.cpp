/* This file is part of the KDE project
  Copyright (C) 2008 Pierre Stirnweiss <pierre.stirnweiss_koffice@gadz.org>

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

#include <kdebug.h>

class KoGenChanges::Private
{
public:
    Private(KoGenChanges *q) : q(q) { }

    /// style definition -> name
    ChangeMap changeMap;

    /// Map with the change name as key.
    /// This map is mainly used to check for name uniqueness
    NameMap changeNames;

    /// List of styles (used to preserve ordering)
    ChangeArray changeArray;
    ChangeMap::iterator insertChange(const KoGenChange &change, const QString &name);

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
    ChangeMap::iterator it = d->changeMap.find(change);
    if (it == d->changeMap.end()) {
        it = d->insertChange(change, name);
    }
    return it.value();
}

KoGenChanges::ChangeMap::iterator KoGenChanges::Private::insertChange(const KoGenChange &change, const QString &name)
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
    changeName = q->makeUniqueName(changeName);
    changeNames.insert(changeName);
    KoGenChanges::ChangeMap::iterator it = changeMap.insert(change, changeName);
    NamedChange s;
    s.change = &it.key();
    s.name = changeName;
    changeArray.append(s);

    return it;
}

KoGenChanges::ChangeMap KoGenChanges::changes() const
{
    return d->changeMap;
}

QString KoGenChanges::makeUniqueName(const QString& base) const
{
    if (! d->changeNames.contains(base))
        return base;
    int num = 1;
    QString name;
    do {
        name = base;
        name += QString::number(num++);
    } while (d->changeNames.contains(name));
    return name;
}

const KoGenChange* KoGenChanges::change(const QString& name) const
{
    ChangeArray::const_iterator it = d->changeArray.constBegin();
    const ChangeArray::const_iterator end = d->changeArray.constEnd();
    for (; it != end ; ++it) {
        if ((*it).name == name)
            return (*it).change;
    }
    return 0;
}

void KoGenChanges::saveOdfChanges(KoXmlWriter* xmlWriter) const
{
    xmlWriter->startElement("text:tracked-changes");

    ChangeMap changesList = changes();
    KoGenChanges::ChangeMap::const_iterator it = changesList.constBegin();
    for (; it != changesList.constEnd() ; ++it) {
        it.key().writeChange(xmlWriter, it.value());
    }

    xmlWriter->endElement(); // text:tracked-changes
}
