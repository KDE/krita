/* This file is part of the KDE project
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef TESTLOADING_H
#define TESTLOADING_H

#include <QObject>

class QScriptEngine;
class QTextDocument;
class QTextEdit;
class KoStore;
class KoTextShapeData;
class KComponentData;

class TestLoading : public QObject 
{
    Q_OBJECT
public:
    TestLoading();
    ~TestLoading();

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void testLoading();
    void testLoading_data();

private:
    QTextDocument *documentFromScript(const QString &script);
    QTextDocument *documentFromOdt(const QString &odt);

    KComponentData *componentData;
    QScriptEngine *engine;
    KoTextShapeData *textShapeData;
    KoStore *store;
};

#endif // TESTLOADING_H
