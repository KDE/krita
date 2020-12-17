/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_IMAGE_ANIMATION_INTERFACE_TEST_H
#define __KIS_IMAGE_ANIMATION_INTERFACE_TEST_H

#include <QtTest/QtTest>
#include <QImage>

#include "kis_types.h"
#include "kis_image.h"

class KisImageAnimationInterfaceTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFrameRegeneration();
    void testFramesChangedSignal();

    void testAnimationCompositionBug();

    void testSwitchFrameWithUndo();
    void testSwitchFrameHangup();


    void slotFrameDone();

private:
    KisImageSP m_image;
    QImage m_compositedFrame;
};

#endif /* __KIS_IMAGE_ANIMATION_INTERFACE_TEST_H */
