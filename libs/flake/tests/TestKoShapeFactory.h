/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Boudewijn Rempt (boud@valdyas.org)
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef TestKoShapeFactoryBase_H
#define TestKoShapeFactoryBase_H

#include <QObject>

class TestKoShapeFactory : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    // tests
    void testCreateFactory();
    void testSupportsKoXmlElement();
    void testPriority();
    void testCreateDefaultShape();
    void testCreateShape();

};

#endif
