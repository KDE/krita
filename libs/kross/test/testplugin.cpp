/***************************************************************************
 * testplugin.cpp
 * This file is part of the KDE project
 * copyright (C)2005 by Sebastian Sauer (mail@dipe.org)
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

#include "testplugin.h"
#include "testobject.h"

/************************************************************************
 * TestPluginObject
 */

TestPluginObject::TestPluginObject(const QString& name)
    : Kross::Api::Class<TestPluginObject>(name)
{
    // Functions to test the basic datatypes
    this->addFunction1< void, Kross::Api::Variant >
        ("voiduintfunc", this, &TestPluginObject::voiduintfunc);
    this->addFunction1< Kross::Api::Variant, Kross::Api::Variant >
        ("uintfunc", this, &TestPluginObject::uintfunc);
    this->addFunction1< Kross::Api::Variant, Kross::Api::Variant >
        ("intfunc", this, &TestPluginObject::intfunc);
    this->addFunction1< Kross::Api::Variant, Kross::Api::Variant >
        ("boolfunc", this, &TestPluginObject::boolfunc);
    this->addFunction1< Kross::Api::Variant, Kross::Api::Variant >
        ("doublefunc", this, &TestPluginObject::doublefunc);
    /*this->addProxyFunction< Kross::Api::Variant, Kross::Api::Variant >
        ("cstringfunc", this, &TestPluginObject::cstringfunc);*/
    this->addFunction1< Kross::Api::Variant, Kross::Api::Variant >
        ("stringfunc", this, &TestPluginObject::stringfunc);
    this->addFunction1< Kross::Api::Variant, Kross::Api::Variant >
        ("stringlistfunc", this, &TestPluginObject::stringlistfunc);
    this->addFunction1< Kross::Api::Variant, Kross::Api::Variant >
        ("variantfunc", this, &TestPluginObject::variantfunc);

    // With 2 arguments
    this->addFunction2< Kross::Api::Variant, Kross::Api::Variant, Kross::Api::Variant >
        ("stringstringfunc", this, &TestPluginObject::stringstringfunc);
    // With 3 arguments
    this->addFunction3< Kross::Api::Variant, Kross::Api::Variant, Kross::Api::Variant, Kross::Api::Variant >
        ("uintdoublestringfunc", this, &TestPluginObject::uintdoublestringfunc);
    // With 4 arguments
    this->addFunction4< Kross::Api::Variant, Kross::Api::Variant, Kross::Api::Variant, Kross::Api::Variant, Kross::Api::Variant >
        ("stringlistbooluintdouble", this, &TestPluginObject::stringlistbooluintdouble);

    // With default arguments
    this->addFunction1< Kross::Api::Variant, Kross::Api::Variant >
        ("uintfunc_defarg", this, &TestPluginObject::uintfunc, new Kross::Api::Variant(12345) );
    this->addFunction1< Kross::Api::Variant, Kross::Api::Variant >
        ("stringfunc_defarg", this, &TestPluginObject::stringfunc, new Kross::Api::Variant("MyDefaultString") );
    this->addFunction1< Kross::Api::Variant, Kross::Api::Variant >
        ("stringlistfunc_defarg", this, &TestPluginObject::stringlistfunc, new Kross::Api::Variant(QVariant(QStringList() << "Default1" << "Default2")));
    this->addFunction1< Kross::Api::Variant, Kross::Api::Variant >
        ("variantfunc_defarg", this, &TestPluginObject::variantfunc, new Kross::Api::Variant("MyDefaultVariantString") );

    // Test passing of objects
    this->addFunction1<TestPluginObject, TestPluginObject>("objectfunc", this, &TestPluginObject::objectfunc, 0);
}

TestPluginObject::~TestPluginObject()
{
}

uint TestPluginObject::uintfunc(uint i) { return i; }
void TestPluginObject::voiduintfunc(uint) {}
int TestPluginObject::intfunc(int i) { return i; }
bool TestPluginObject::boolfunc(bool b) { return b; }
double TestPluginObject::doublefunc(double d) { return d; }
QString TestPluginObject::stringfunc(const QString& s) { return s; }
QStringList TestPluginObject::stringlistfunc(const QStringList& sl) { return sl; }
QVariant TestPluginObject::variantfunc(const QVariant& v) { return v; }
TestPluginObject* TestPluginObject::objectfunc(TestPluginObject* obj) { return obj; }
QString TestPluginObject::stringstringfunc(const QString& s, const QString&) { return s; }
uint TestPluginObject::uintdoublestringfunc(uint i, double, const QString&) { return i; }
QStringList TestPluginObject::stringlistbooluintdouble(const QStringList& sl, bool, uint, double) { return sl; }

/************************************************************************
 * TestPluginModule
 */

TestPluginModule::TestPluginModule(const QString& name)
    : Kross::Api::Module(name)
    , m_testobject( new TestObject() )

{
    addChild(new TestPluginObject("testpluginobject1"));

    // Let's wrap a whole instance and it's methodfunctions.
    Kross::Api::Class<TestObject> *testobjectclass =
        new Kross::Api::Class<TestObject>("testpluginobject2");
    addChild(testobjectclass);

    // Wrap a whole QObject
    addChild( new Kross::Api::QtObject( new TestObject() , "testqobject1" ) );
}

TestPluginModule::~TestPluginModule()
{
    delete m_testobject;
}

