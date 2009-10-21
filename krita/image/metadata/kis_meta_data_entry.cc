/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_meta_data_entry.h"
#include <QString>

#include <kis_debug.h>

#include "kis_meta_data_value.h"
#include "kis_meta_data_schema.h"

using namespace KisMetaData;

struct Entry::Private {
    QString name;
    const Schema* schema;
    Value value;
    bool valid;
};

Entry::Entry() :
        d(new Private)
{
    d->name = "";
    d->schema = 0;
    d->valid = false;
}

Entry::Entry(const Schema* schema, QString name, const Value& value) :
        d(new Private)
{
    Q_ASSERT(!name.isEmpty());
    Q_ASSERT(isValidName(name));
    d->name = name;
    d->schema = schema;
    d->value = value;
    d->valid = true;
}

Entry::Entry(const Entry& e) : d(new Private())
{
    d->valid = false;
    *this = e;
}

Entry::~Entry()
{
    delete d;
}

QString Entry::name() const
{
    return d->name;
}

const Schema* Entry::schema() const
{
    Q_ASSERT(d->schema);
    return d->schema;
}

void Entry::setSchema(const KisMetaData::Schema* schema)
{
    d->schema = schema;
}

QString Entry::qualifiedName() const
{
    return d->schema->generateQualifiedName(d->name);
}

const Value& Entry::value() const
{
    return d->value;
}

Value& Entry::value()
{
    return d->value;
}

bool Entry::isValid() const
{
    return d->valid;
}

bool Entry::isValidName(const QString& _name)
{
    if (_name.length() < 1) {
        dbgImage << "Too small";
        return false;
    }
    if (!_name[0].isLetter()) {
        dbgImage << _name << " doesn't start by a letter";
        return false;
    }
    for (int i = 1; i < _name.length(); ++i) {
        QChar c = _name[i];
        if (!c.isLetterOrNumber()) {
            dbgImage << _name << " " << i << "th character isn't a letter or a digit";
            return false;
        }
    }
    return true;
}


bool Entry::operator==(const Entry& e)
{
    return qualifiedName() == e.qualifiedName();
}

Entry& Entry::operator=(const Entry & e)
{
    if (e.isValid()) {
        Q_ASSERT(!isValid() || *this == e);
        d->name = e.d->name;
        d->schema = e.d->schema;
        d->value = e.d->value;
        d->valid = true;
    }
    return *this;
}

QDebug operator<<(QDebug debug, const Entry &c)
{
    debug.nospace() << "Name: " << c.name() << " Qualified name: " << c.qualifiedName() << " Value: " << c.value();
    return debug.space();
}
