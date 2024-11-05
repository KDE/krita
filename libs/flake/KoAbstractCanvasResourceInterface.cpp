/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoAbstractCanvasResourceInterface.h"

KoAbstractCanvasResourceInterface::KoAbstractCanvasResourceInterface(int key, const QString debugTag)
    : m_key(key)
    , m_debugTag(debugTag)
{
}

int KoAbstractCanvasResourceInterface::key() const {
    return m_key;
}
