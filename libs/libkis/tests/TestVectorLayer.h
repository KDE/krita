/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2017 Boudewijn Rempt

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef TESTVECTORLAYER_H
#define TESTVECTORLAYER_H

#include <QObject>
#include <KisDocument.h>
#include <VectorLayer.h>
#include <kis_shape_layer.h>



class TestVectorLayer : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testAddShapesFromSvg();
    void testShapeAtPosition();
    void testShapesInRect();
    void testCreateGroupShape();
    void cleanupTestCase();

private:
    KisDocument* kisdoc;
    VectorLayer *vNode;
    KisImageSP image;
    KisShapeLayerSP vLayer1;
};
#endif
