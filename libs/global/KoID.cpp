/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoID.h"

KoID::TranslatedString::TranslatedString(
    const boost::optional<KLocalizedString> &source)
    : QString(!source->isEmpty() ? source->toString() : QString())
{
}

KoID::TranslatedString::TranslatedString(const QString &value)
    : QString(value)
{
}

KoID::KoIDPrivate::KoIDPrivate(QString _id, const KLocalizedString &_name)
    : id(std::move(_id))
    , name(_name)
{
}

KoID::KoIDPrivate::KoIDPrivate(QString _id, const QString &_name)
    : id(std::move(_id))
    , name(StorageType::init_value_tag(), _name)
{
}

KoID::KoID()
    : m_d(new KoIDPrivate(QString(), QString()))
{
}

KoID::KoID(const QString &id, const QString &name)
    : m_d(new KoIDPrivate(id, name))
{
}

KoID::KoID(const QString &id, const KLocalizedString &name)
    : m_d(new KoIDPrivate(id, name))
{
}

KoID::KoID(const KoID &rhs)
    : m_d(rhs.m_d)
{
}

KoID &KoID::operator=(const KoID &rhs)
{
    if (this != &rhs) {
        m_d = rhs.m_d;
    }
    return *this;
}

QString KoID::id() const
{
    return m_d->id;
}

QString KoID::name() const
{
    return *m_d->name;
}
