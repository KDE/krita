/* This file is part of the KDE project
   Copyright (C) 2003-2012 Jarosław Staniek <staniek@kde.org>

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

#include "fieldlist.h"
#include "object.h"

#include <kdebug.h>

#include <assert.h>

using namespace KexiDB;

FieldList::FieldList(bool owner)
        : m_fields(owner)
        , m_autoinc_fields(0)
{
}

FieldList::FieldList(const FieldList& fl, bool deepCopyFields)
        : m_fields(fl.m_fields.autoDelete())
        , m_autoinc_fields(0)
{
    if (deepCopyFields) {
        //deep copy for the fields
        foreach(Field *origField, fl.m_fields) {
            Field *f = origField->copy();
            if (origField->m_parent == &fl)
                f->m_parent = this;
            addField(f);
        }
    }
}

FieldList::~FieldList()
{
    delete m_autoinc_fields;
}

void FieldList::clear()
{
// m_name.clear();
    m_fields_by_name.clear();
    delete m_autoinc_fields;
    m_autoinc_fields = 0;
    m_fields.clear();
    m_sqlFields.clear();
}

FieldList& FieldList::insertField(uint index, KexiDB::Field *field)
{
    assert(field);
    if (!field)
        return *this;
    if (index > (uint)m_fields.count()) {
        KexiDBFatal << "FieldList::insertField(): index (" << index << ") out of range";
        return *this;
    }
    m_fields.insert(index, field);
    if (!field->name().isEmpty())
        m_fields_by_name.insert(field->name().toLower(), field);
    m_sqlFields.clear();
    return *this;
}

void FieldList::renameField(const QString& oldName, const QString& newName)
{
    Field *field = m_fields_by_name.value(oldName.toLower());
    if (!field) {
        KexiDBFatal << "FieldList::renameField() no field found "
        << QString("\"%1\"").arg(oldName);
        return;
    }
    renameFieldInternal(field, newName.toLower());
}

void FieldList::renameField(KexiDB::Field *field, const QString& newName)
{
    if (!field || field != m_fields_by_name.value(field->name().toLower())) {
        KexiDBFatal << "FieldList::renameField() no field found "
        << (field ? QString("\"%1\"").arg(field->name()) : QString());
        return;
    }
    renameFieldInternal(field, newName.toLower());
}

void FieldList::renameFieldInternal(KexiDB::Field *field, const QString& newNameLower)
{
    m_fields_by_name.remove(field->name().toLower());
    field->setName(newNameLower);
    m_fields_by_name.insert(newNameLower, field);
}


FieldList& FieldList::addField(KexiDB::Field *field)
{
    return insertField(m_fields.count(), field);
}

void FieldList::removeField(KexiDB::Field *field)
{
    assert(field);
    if (!field)
        return;
    m_fields_by_name.remove(field->name().toLower());
    m_fields.removeAt(m_fields.indexOf(field));
    m_sqlFields.clear();
}

Field* FieldList::field(const QString& name)
{
    return m_fields_by_name.value(name.toLower());
}

QString FieldList::debugString()
{
    if (m_fields.isEmpty())
        return "<NO FIELDS>";
    QString dbg;
    dbg.reserve(512);
    bool start = true;
    foreach(Field *field, m_fields) {
        if (!start)
            dbg += ",\n";
        else
            start = false;
        dbg += "  ";
        dbg += field->debugString();
    }
    return dbg;
}

void FieldList::debug()
{
    KexiDBDbg << debugString();
}

#define _ADD_FIELD(fname) \
    { \
        if (fname.isEmpty()) return fl; \
        f = m_fields_by_name.value(fname.toLower()); \
        if (!f) { KexiDBWarn << subListWarning1(fname); delete fl; return 0; } \
        fl->addField(f); \
    }

static QString subListWarning1(const QString& fname)
{
    return QString("FieldList::subList() could not find field \"%1\"").arg(fname);
}

FieldList* FieldList::subList(const QString& n1, const QString& n2,
                              const QString& n3, const QString& n4,
                              const QString& n5, const QString& n6,
                              const QString& n7, const QString& n8,
                              const QString& n9, const QString& n10,
                              const QString& n11, const QString& n12,
                              const QString& n13, const QString& n14,
                              const QString& n15, const QString& n16,
                              const QString& n17, const QString& n18)
{
    if (n1.isEmpty())
        return 0;
    Field *f;
    FieldList *fl = new FieldList(false);
    _ADD_FIELD(n1);
    _ADD_FIELD(n2);
    _ADD_FIELD(n3);
    _ADD_FIELD(n4);
    _ADD_FIELD(n5);
    _ADD_FIELD(n6);
    _ADD_FIELD(n7);
    _ADD_FIELD(n8);
    _ADD_FIELD(n9);
    _ADD_FIELD(n10);
    _ADD_FIELD(n11);
    _ADD_FIELD(n12);
    _ADD_FIELD(n13);
    _ADD_FIELD(n14);
    _ADD_FIELD(n15);
    _ADD_FIELD(n16);
    _ADD_FIELD(n17);
    _ADD_FIELD(n18);
    return fl;
}

FieldList* FieldList::subList(const QStringList& list)
{
    Field *f;
    FieldList *fl = new FieldList(false);
    for (QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it) {
        _ADD_FIELD((*it));
    }
    return fl;
}

FieldList* FieldList::subList(const QList<uint>& list)
{
    Field *f;
    FieldList *fl = new FieldList(false);
    foreach(uint index, list) {
        f = field(index);
        if (!f) {
            KexiDBWarn << QString("FieldList::subList() could not find field at position %1").arg(index);
            delete fl;
            return 0;
        }
        fl->addField(f);
    }
    return fl;
}

QStringList FieldList::names() const
{
    QStringList r;
// for (QDictIterator<Field> it(m_fields_by_name);it.current();++it) {
//  r += it.currentKey().toLower();
// }
    foreach(Field *f, m_fields) {
        r += f->name().toLower();
    }
    return r;
}

//static
QString FieldList::sqlFieldsList(Field::List* list, const Driver *driver,
                                 const QString& separator, const QString& tableAlias, int drvEscaping)
{
    if (!list)
        return QString();
    QString result;
    result.reserve(256);
    bool start = true;
    const QString tableAliasAndDot(tableAlias.isEmpty() ? QString() : (tableAlias + "."));
    int kexiEscaping = drvEscaping | Driver::EscapeDriver;
    kexiEscaping ^= Driver::EscapeDriver;
    kexiEscaping |= Driver::EscapeKexi;
    foreach(Field *f, *list) {
        if (!start)
            result += separator;
        else
            start = false;
        result += (tableAliasAndDot + escapeIdentifier(driver, f->name(), drvEscaping));
    }
    return result;
}

QString FieldList::sqlFieldsList(const Driver *driver,
                                 const QString& separator, const QString& tableAlias, int drvEscaping)
{
    if (!m_sqlFields.isEmpty())
        return m_sqlFields;

    m_sqlFields = FieldList::sqlFieldsList(&m_fields, driver, separator, tableAlias, drvEscaping);
    return m_sqlFields;
}

Field::List* FieldList::autoIncrementFields()
{
    if (m_autoinc_fields)
        return m_autoinc_fields;

    m_autoinc_fields = new Field::List();
    m_autoinc_fields->setAutoDelete(false);
    foreach(Field *f, m_fields) {
        if (f->isAutoIncrement()) {
            m_autoinc_fields->append(f);
        }
    }
    return m_autoinc_fields;
}
