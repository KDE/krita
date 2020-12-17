/*
 *  SPDX-FileCopyrightText: 2009 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_projection_backend.h"

KisProjectionBackend::~KisProjectionBackend()
{
}

void KisProjectionBackend::alignSourceRect(QRect& rect, qreal scale)
{
    Q_UNUSED(rect);
    Q_UNUSED(scale);
}
