/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_ANIMATION_FRAME_CACHE_TEST_H
#define KIS_ANIMATION_FRAME_CACHE_TEST_H

#include <QtTest>

class KisAnimationFrameCache;

class KisAnimationFrameCacheTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCache();

    void slotFrameGerenationFinished(int time);

private:
    KisAnimationFrameCache *m_globalAnimationCache = 0;

};
#endif

