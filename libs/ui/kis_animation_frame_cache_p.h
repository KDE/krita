/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_ANIMATION_FRAME_CACHE_P_H
#define KIS_ANIMATION_FRAME_CACHE_P_H

#include <QMap>
#include <kis_time_span.h>
#include <kritaui_export.h>

struct KRITAUI_EXPORT FramesGluerBase
{
    FramesGluerBase(QMap<int, int> &_frames) : frames(_frames) {}

    QMap<int, int> &frames;

    bool glueFrames(const KisTimeSpan &range);

    virtual ~FramesGluerBase();
    virtual void moveFrame(int oldStart, int newStart) = 0;
    virtual void forgetFrame(int start) = 0;
};

#endif // KIS_ANIMATION_FRAME_CACHE_P_H
