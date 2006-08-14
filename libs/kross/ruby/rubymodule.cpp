/***************************************************************************
 * rubyinterpreter.cpp
 * This file is part of the KDE project
 * copyright (C)2005 by Cyrille Berger (cberger@cberger.net)
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

#include "rubymodule.h"

#include "rubyconfig.h"
#include "rubyextension.h"

namespace Kross {

class RubyModulePrivate {
    friend class RubyModule;

#if 0
    /// The \a Kross::Module this RubyExtension wraps.
    Kross::Module::Ptr m_module;
#endif
};

RubyModule::RubyModule(/*Kross::Module::Ptr mod,*/ QString modname) : d(new RubyModulePrivate)
{
#if 0
    d->m_module = mod;
#endif
    modname = modname.left(1).toUpper() + modname.right(modname.length() - 1 );
    krossdebug(QString("Module: %1").arg(modname));
#if 0
    VALUE rmodule = rb_define_module(modname.toAscii());
    rb_define_module_function(rmodule,"method_missing",  (VALUE (*)(...))RubyModule::method_missing, -1);
    VALUE rm = RubyExtension::toVALUE( mod.data() );
    rb_define_const(rmodule, "MODULEOBJ", rm);
#endif
}

RubyModule::~RubyModule()
{
}

VALUE RubyModule::method_missing(int argc, VALUE *argv, VALUE self)
{
    #ifdef KROSS_RUBY_MODULE_DEBUG
        QString funcname = rb_id2name(SYM2ID(argv[0]));
        krossdebug(QString("Function %1 missing in a module").arg(funcname));
    #endif
#if 0
    VALUE rubyObjectModule = rb_funcall( self, rb_intern("const_get"), 1, ID2SYM(rb_intern("MODULEOBJ")) );
    RubyModule* objectModule;
    Data_Get_Struct(rubyObjectModule, RubyModule, objectModule);
    Kross::Object::Ptr object = Kross::Object::Ptr( dynamic_cast<Kross::Object*>( objectModule->d->m_module.data() ) );
    return RubyExtension::call_method(object, argc, argv);
#endif
    return Qfalse;
}

}
