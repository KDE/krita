/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoLocalStrokeCanvasResources.h"

#include <QVariant>
#include <QMap>


struct KoLocalStrokeCanvasResources::Private
{
    QMap<int, QVariant> resources;
};

KoLocalStrokeCanvasResources::KoLocalStrokeCanvasResources()
    : m_d(new Private)
{
}

KoLocalStrokeCanvasResources::KoLocalStrokeCanvasResources(const KoLocalStrokeCanvasResources &rhs)
    : m_d(new Private)
{
    m_d->resources = rhs.m_d->resources;
}

KoLocalStrokeCanvasResources &KoLocalStrokeCanvasResources::operator=(const KoLocalStrokeCanvasResources &rhs)
{
    m_d->resources = rhs.m_d->resources;
    return *this;
}

KoLocalStrokeCanvasResources::~KoLocalStrokeCanvasResources()
{
}


QVariant KoLocalStrokeCanvasResources::resource(int key) const
{
    return m_d->resources.value(key, QVariant());
}

void KoLocalStrokeCanvasResources::storeResource(int key, const QVariant &resource)
{
    m_d->resources[key] = resource;
}
