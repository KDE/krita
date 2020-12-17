/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoActiveCanvasResourceDependency.h"

struct KoActiveCanvasResourceDependency::Private
{
    Private(int _sourceKey, int _targetKey)
        : sourceKey(_sourceKey),
          targetKey(_targetKey)
    {
    }

    int sourceKey = -1;
    int targetKey = -1;
};

KoActiveCanvasResourceDependency::KoActiveCanvasResourceDependency(int sourceKey, int targetKey)
    : m_d(new Private(sourceKey, targetKey))
{
}

KoActiveCanvasResourceDependency::~KoActiveCanvasResourceDependency()
{
}

int KoActiveCanvasResourceDependency::sourceKey() const
{
    return m_d->sourceKey;
}

int KoActiveCanvasResourceDependency::targetKey() const
{
    return m_d->targetKey;
}
