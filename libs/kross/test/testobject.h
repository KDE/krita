/***************************************************************************
 * testobject.h
 * This file is part of the KDE project
 * copyright (C)2004-2005 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KROSS_TEST_TESTOBJECT_H
#define KROSS_TEST_TESTOBJECT_H

#include "../main/scriptcontainer.h"

#include <qobject.h>
#include <qstring.h>

class TestObject : public QObject
{
        Q_OBJECT

        //Q_PROPERTY(QString testProperty READ testProperty WRITE setTestProperty)

    public:
        TestObject();
        TestObject(QObject* parent, Kross::Api::ScriptContainer::Ptr scriptcontainer);
        ~TestObject();

        uint func1(uint);
        void func2(QString, int);
        QString func3(QString, int);
        const QString& func4(const QString&, int) const;

        //QString m_prop;
        //QString testProperty() const { return m_prop; }
        //void setTestProperty(QString prop) { m_prop = prop; }

    signals:
        void testSignal();
        void testSignalString(const QString&);
        void stdoutSignal(const QString&);
        void stderrSignal(const QString&);
    public slots:
        void testSlot();
        void testSlot2();
        void stdoutSlot(const QString&);
        void stderrSlot(const QString&);
    private slots:
        void testSignalSlot();
};

#endif
