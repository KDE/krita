/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    bool invisibleChangeHappened = false;
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

    const bool valueChanged = m_d->lastKnownValue != newValue || m_d->invisibleChangeHappened;
    m_d->lastKnownValue = newValue;
    m_d->invisibleChangeHappened = false;

    return valueChanged;
}

QVariant KoDerivedResourceConverter::readFromSource(const QVariant &sourceValue)
{
    const QVariant result = fromSource(sourceValue);
    m_d->invisibleChangeHappened |= result != m_d->lastKnownValue;
    m_d->lastKnownValue = result;
    return m_d->lastKnownValue;
}

QVariant KoDerivedResourceConverter::writeToSource(const QVariant &value,
                                                   const QVariant &sourceValue,
                                                   bool *changed)
{
    QVariant newSourceValue = sourceValue;
    const bool hasChanged = m_d->lastKnownValue != value || m_d->invisibleChangeHappened;
    m_d->invisibleChangeHappened = false;

    if (hasChanged || value != fromSource(sourceValue)) {
        newSourceValue = toSource(value, sourceValue);
        /**
         * Some resources may be immutable, that is, writing to them will
         * **not** alter the value. Example: size property of the Shape Brush
         * (always 1.0)
         */
        m_d->lastKnownValue = fromSource(newSourceValue);
    }
    if (changed) {
        *changed = hasChanged;
    }
    return newSourceValue;
}
