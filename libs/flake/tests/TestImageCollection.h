/*
 *  This file is part of Calligra tests
 *
 *  SPDX-FileCopyrightText: 2006-2010 Thomas Zander <zander@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TESTIMAGECOLLECTION_H
#define TESTIMAGECOLLECTION_H

#include <QObject>

class TestImageCollection : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    // tests
    void testGetImageImage();
    void testGetImageStore();
    void testInvalidImageData();

    // imageData tests
    void testImageDataAsSharedData();
    void testPreload1();
    void testPreload3();
    void testSameKey();
    void testIsValid();
};

#endif /* TESTIMAGECOLLECTION_H */
