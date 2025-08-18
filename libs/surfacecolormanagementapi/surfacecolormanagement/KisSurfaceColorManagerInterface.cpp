/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisSurfaceColorManagerInterface.h"

KisSurfaceColorManagerInterface::KisSurfaceColorManagerInterface(QWindow *window)
    : m_window(window)
{
}

KisSurfaceColorManagerInterface::~KisSurfaceColorManagerInterface()
{
}

#include <moc_KisSurfaceColorManagerInterface.cpp>