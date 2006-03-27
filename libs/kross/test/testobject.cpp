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

#include <kdebug.h>

TestObject::TestObject()
    : QObject(0, "TestObject")
{
}

TestObject::TestObject(QObject* parent, Kross::Api::ScriptContainer::Ptr scriptcontainer)
    : QObject(parent, "TestObject")
{
    //kDebug() << "TestObject::TestObject Constructor." << endl;

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
    //kDebug() << "TestObject::~TestObject Destructor." << endl;
}

uint TestObject::func1(uint i)
{
    kDebug() << "CALLED => TestObject::func1 i=" << i << endl;
    return i;
}

void TestObject::func2(QString s, int i)
{
    kDebug() << "CALLED => TestObject::func2 s=" << s << " i=" << i << endl;
}

QString TestObject::func3(QString s, int i)
{
    kDebug() << "CALLED => TestObject::func3 s=" << s << " i=" << i << endl;
    return s;
}

const QString& TestObject::func4(const QString& s, int i) const
{
    kDebug() << "CALLED => TestObject::func4 s=" << s << " i=" << i << endl;
    return s;
}

void TestObject::testSlot()
{
    kDebug() << "TestObject::testSlot called" << endl;
    emit testSignal();
    emit testSignalString("This is the emitted TestObject::testSignalString(const QString&)");
}

void TestObject::testSlot2()
{
    kDebug() << "TestObject::testSlot2 called" << endl;
}

void TestObject::testSignalSlot()
{
    kDebug() << "TestObject::testSignalSlot called" << endl;
}

void TestObject::stdoutSlot(const QString& s)
{
    kDebug() << "<stdout> " << s << endl;
    //std::cout << "<stdout> " << s.latin1() << std::endl;
}

void TestObject::stderrSlot(const QString& s)
{
    kDebug() << "<stderr> " << s << endl;
    //std::cout << "<stderr> " << s.latin1() << std::endl;
}

//#include "testobject.moc"
