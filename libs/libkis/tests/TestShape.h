/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2017 Boudewijn Rempt

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef TESTSHAPE_H
#define TESTSHAPE_H

#include <QObject>
#include <KisDocument.h>
#include <kis_shape_layer.h>
#include <VectorLayer.h>

class TestShape : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testParentShape();
    void cleanupTestCase();
private:
    KisDocument* kisdoc;
    VectorLayer *vNode;
    KisImageSP image;
    KisShapeLayerSP vLayer1;
};

#endif
