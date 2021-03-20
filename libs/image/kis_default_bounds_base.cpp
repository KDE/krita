/*
 *  SPDX-FileCopyrightText: 2010 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_default_bounds_base.h"

KisDefaultBoundsBase::~KisDefaultBoundsBase()
{
}

QRect KisDefaultBoundsBase::imageBorderRect() const
{
    return bounds();
}

