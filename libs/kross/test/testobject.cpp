/***************************************************************************
 * testobject.cpp
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

#include "testobject.h"

#include <iostream> // for std::out

TestObject::TestObject()
    : QObject(0, "TestObject")
{
}

TestObject::TestObject(QObject* parent, Kross::Api::ScriptContainer::Ptr scriptcontainer)
    : QObject(parent, "TestObject")
{
    connect(this, SIGNAL(testSignal()), this, SLOT(testSignalSlot()));
    connect(this, SIGNAL(stdoutSignal(const QString&)), this, SLOT(stdoutSlot(const QString&)));
    connect(this, SIGNAL(stderrSignal(const QString&)), this, SLOT(stderrSlot(const QString&)));

    scriptcontainer->addQObject(this);

//scriptcontainer->addSignal("stdout", this, SIGNAL(stdoutSignal(const QString&)));
//scriptcontainer->addSlot("stderr", this, SLOT(stderrSlot(const QString&)));

    //scriptcontainer->addSignal("myTestSignal", this, SIGNAL(testSignal()));
    //scriptcontainer->addSlot("myTestSlot", this, SLOT(testSlot()));
}

TestObject::~TestObject()
{
}

uint TestObject::func1(uint i)
{
    Kross::krossdebug(QString("CALLED => TestObject::func1 i=%1").arg(i) );
    return i;
}

void TestObject::func2(QString s, int i)
{
    Kross::krossdebug(QString("CALLED => TestObject::func2 s=%1 i=%2").arg(s).arg(i));
}

QString TestObject::func3(QString s, int i)
{
    Kross::krossdebug(QString("CALLED => TestObject::func3 s=%1 i=%2").arg(s).arg(i));
    return s;
}

const QString& TestObject::func4(const QString& s, int i) const
{
    Kross::krossdebug(QString("CALLED => TestObject::func4 s=%1 i=%2").arg(s).arg(i));
    return s;
}

void TestObject::testSlot()
{
    Kross::krossdebug("TestObject::testSlot called");
    emit testSignal();
    emit testSignalString("This is the emitted TestObject::testSignalString(const QString&)");
}

void TestObject::testSlot2()
{
    Kross::krossdebug("TestObject::testSlot2 called");
}

void TestObject::testSignalSlot()
{
    Kross::krossdebug("TestObject::testSignalSlot called");
}

void TestObject::stdoutSlot(const QString& s)
{
    Kross::krossdebug(QString("stdout: %1").arg(s));
    //std::cout << "<stdout> " << s.latin1() << std::endl;
}

void TestObject::stderrSlot(const QString& s)
{
    Kross::krossdebug(QString("stderr: %1").arg(s));
    //std::cout << "<stderr> " << s.latin1() << std::endl;
}

//#include "testobject.moc"
