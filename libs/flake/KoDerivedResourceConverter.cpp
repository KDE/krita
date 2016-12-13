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

#include "KoDerivedResourceConverter.h"

#include "QVariant"
#include "kis_assert.h"

struct KoDerivedResourceConverter::Private
{
    Private(int _key, int _sourceKey)
        : key(_key), sourceKey(_sourceKey) {}

    int key;
    int sourceKey;

    QVariant lastKnownValue;
};


KoDerivedResourceConverter::KoDerivedResourceConverter(int key, int sourceKey)
    : m_d(new Private(key, sourceKey))
{
}

KoDerivedResourceConverter::~KoDerivedResourceConverter()
{
}

int KoDerivedResourceConverter::key() const
{
    return m_d->key;
}

int KoDerivedResourceConverter::sourceKey() const
{
    return m_d->sourceKey;
}

bool KoDerivedResourceConverter::notifySourceChanged(const QVariant &sourceValue)
{
    const QVariant newValue = fromSource(sourceValue);

    const bool valueChanged = m_d->lastKnownValue != newValue;
    m_d->lastKnownValue = newValue;

    return valueChanged;
}

QVariant KoDerivedResourceConverter::readFromSource(const QVariant &sourceValue)
{
    const QVariant result = fromSource(sourceValue);

    KIS_SAFE_ASSERT_RECOVER_NOOP(m_d->lastKnownValue.isNull() ||
                                 result == m_d->lastKnownValue);

    m_d->lastKnownValue = result;

    return m_d->lastKnownValue;
}

QVariant KoDerivedResourceConverter::writeToSource(const QVariant &value,
                                                   const QVariant &sourceValue,
                                                   bool *changed)
{
    QVariant newSourceValue = sourceValue;
    bool hasChanged = m_d->lastKnownValue != value;
    if (hasChanged || value != fromSource(sourceValue)) {
        m_d->lastKnownValue = value;
        newSourceValue = toSource(value, sourceValue);
    }
    if (changed) {
        *changed = hasChanged;
    }
    return newSourceValue;
}
