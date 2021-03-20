/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISQPAINTERSTATESAVER_H
#define KISQPAINTERSTATESAVER_H

#include "kritaglobal_export.h"

class QPainter;

class KRITAGLOBAL_EXPORT KisQPainterStateSaver
{
public:
    KisQPainterStateSaver(QPainter *painter);
    ~KisQPainterStateSaver();

private:
    KisQPainterStateSaver(const KisQPainterStateSaver &rhs);
    QPainter *m_painter;
};

#endif // KISQPAINTERSTATESAVER_H
