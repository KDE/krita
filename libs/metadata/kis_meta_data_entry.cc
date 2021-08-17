/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "kis_meta_data_entry.h"
#include <QString>

#include <kis_debug.h>

#include "kis_meta_data_value.h"
#include "kis_meta_data_schema.h"

using namespace KisMetaData;

struct Q_DECL_HIDDEN Entry::Private {
    QString name;
    const Schema* schema;
    Value value;
    bool valid;
};

Entry::Entry() :
        d(new Private)
{
    d->schema = 0;
    d->valid = false;
}

Entry::Entry(const Schema* schema, QString name, const Value& value) :
        d(new Private)
{
    Q_ASSERT(!name.isEmpty());
    if (!isValidName(name)) {
        errKrita << "Invalid metadata name:" << name;
        d->name = QString("INVALID: %1").arg(name);
    }
    else {
        d->name = name;
    }
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
    Q_ASSERT(schema);
    d->schema = schema;
}

QString Entry::qualifiedName() const
{
    Q_ASSERT(d->schema);
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
        dbgMetaData << "Too small";
        return false;
    }
    if (!_name[0].isLetter()) {
        dbgMetaData << _name << " doesn't start by a letter";
        return false;
    }
    for (int i = 1; i < _name.length(); ++i) {
        QChar c = _name[i];
        if (!c.isLetterOrNumber()) {
            dbgMetaData << _name << " " << i << "th character isn't a letter or a digit";
            return false;
        }
    }
    return true;
}


bool Entry::operator==(const Entry& e) const
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
