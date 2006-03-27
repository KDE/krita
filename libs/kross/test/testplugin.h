/***************************************************************************
 * testplugin.h
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

#ifndef KROSS_TEST_TESTPLUGIN_H
#define KROSS_TEST_TESTPLUGIN_H

#include "../api/object.h"
#include "../api/list.h"
#include "../api/class.h"
#include "../api/proxy.h"
#include "../api/module.h"

#include <qobject.h>
#include <qstring.h>

class TestPluginObject : public Kross::Api::Class<TestPluginObject>
{
    public:
        TestPluginObject(const QString& name);
        virtual ~TestPluginObject();
        virtual const QString getClassName() const;

    private:
        uint internalfunc1(uint);

        Kross::Api::Object::Ptr func1(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr func2(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr func3(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr func4(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr func5(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr func6(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr func7(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr func8(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr func9(Kross::Api::List::Ptr);
};

class TestObject;

class TestPluginModule : public Kross::Api::Module
{
    public:
        TestPluginModule(const QString& name);
        virtual ~TestPluginModule();
        virtual const QString getClassName() const;

        virtual Kross::Api::Object::Ptr get(const QString& /*name*/, void* /*pointer*/ = 0)
        {
                return 0;
        }
    private:
        TestObject* m_testobject;
};

#endif
