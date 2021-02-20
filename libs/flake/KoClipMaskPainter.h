/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOCLIPMASKPAINTER_H
#define KOCLIPMASKPAINTER_H

#include "kritaflake_export.h"

#include <QScopedPointer>

class QPainter;
class QRectF;


class KRITAFLAKE_EXPORT KoClipMaskPainter
{
public:
    KoClipMaskPainter(QPainter *painter, const QRectF &globalClipRect);
    ~KoClipMaskPainter();

    QPainter* shapePainter();
    QPainter* maskPainter();

    void renderOnGlobalPainter();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KOCLIPMASKPAINTER_H
