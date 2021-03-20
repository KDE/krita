/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisQPainterStateSaver.h"

#include <QPainter>

KisQPainterStateSaver::KisQPainterStateSaver(QPainter *painter)
    : m_painter(painter)
{
    m_painter->save();
}

KisQPainterStateSaver::~KisQPainterStateSaver()
{
    m_painter->restore();
}

