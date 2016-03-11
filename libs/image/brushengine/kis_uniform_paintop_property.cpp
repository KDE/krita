/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_uniform_paintop_property.h"

#include <QVariant>
#include "kis_debug.h"

struct KisUniformPaintOpProperty::Private
{
    Private(Type _type,
            const QString &_id,
            const QString &_name) : type(_type), id(_id), name(_name) {}

    Type type;
    QString id;
    QString name;

    QVariant value;
};

KisUniformPaintOpProperty::KisUniformPaintOpProperty(Type type,
                                                     const QString &id,
                                                     const QString &name,
                                                     QObject *parent)
    : QObject(parent),
      m_d(new Private(type, id, name))
{
}

KisUniformPaintOpProperty::~KisUniformPaintOpProperty()
{
}

QString KisUniformPaintOpProperty::id() const
{
    return m_d->id;
}

QString KisUniformPaintOpProperty::name() const
{
    return m_d->name;
}

KisUniformPaintOpProperty::Type KisUniformPaintOpProperty::type() const
{
    return m_d->type;
}

int KisUniformPaintOpProperty::valueInt() const
{
    KIS_ASSERT_RECOVER(m_d->type == Int) { return 0; }
    return m_d->value.toInt();
}

qreal KisUniformPaintOpProperty::valueDouble() const
{
    KIS_ASSERT_RECOVER(m_d->type == Double) { return 0; }
    return m_d->value.toReal();
}

bool KisUniformPaintOpProperty::valueBool() const
{
    KIS_ASSERT_RECOVER(m_d->type == Bool) { return 0; }
    return m_d->value.toBool();
}

int KisUniformPaintOpProperty::valueCombo() const
{
    KIS_ASSERT_RECOVER(m_d->type == Combo) { return 0; }
    return m_d->value.toInt();
}

void KisUniformPaintOpProperty::setValueInt(int value)
{
    KIS_ASSERT_RECOVER_RETURN(m_d->type == Int);
    if (m_d->value == value) return;

    m_d->value = value;
    emit sigValueIntChanged(value);
}

void KisUniformPaintOpProperty::setValueDouble(qreal value)
{
    KIS_ASSERT_RECOVER_RETURN(m_d->type == Double);
    if (m_d->value == value) return;

    m_d->value = value;
    emit sigValueDoubleChanged(value);
}

void KisUniformPaintOpProperty::setValueBool(bool value)
{
    KIS_ASSERT_RECOVER_RETURN(m_d->type == Bool);
    if (m_d->value == value) return;

    m_d->value = value;
    emit sigValueBoolChanged(value);
}

void KisUniformPaintOpProperty::setValueCombo(int value)
{
    KIS_ASSERT_RECOVER_RETURN(m_d->type == Combo);
    if (m_d->value == value) return;

    m_d->value = value;
    emit sigValueComboChanged(value);
}

