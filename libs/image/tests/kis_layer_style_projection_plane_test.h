/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_LAYER_STYLE_PROJECTION_PLANE_TEST_H
#define __KIS_LAYER_STYLE_PROJECTION_PLANE_TEST_H

#include <QtTest/QtTest>

#include "kis_types.h"

#include <kis_psd_layer_style.h>

class KisLayerStyleProjectionPlaneTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testShadow();
    void testGlow();
    void testGlowGradient();
    void testGlowGradientJitter();
    void testGlowInnerGradient();

    void testSatin();
    void testColorOverlay();
    void testGradientOverlay();
    void testPatternOverlay();

    void testStroke();

    void testBumpmap();

    void testBevel();

    void testBlending();

private:
    void test(KisPSDLayerStyleSP style, const QString testName);

};

#endif /* __KIS_LAYER_STYLE_PROJECTION_PLANE_TEST_H */
