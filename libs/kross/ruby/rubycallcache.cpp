/***************************************************************************
 * rubycallcache.cpp
 * This file is part of the KDE project
 * copyright (C)2006 by Cyrille Berger (cberger@cberger.net)
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

#include "rubycallcache.h"


#include <QVariant>
        
#include <rubyvariant.h>
        
namespace Kross {
    struct RubyCallCachePrivate {
        RubyCallCachePrivate(QObject* nobject, int nmethodindex, bool nhasreturnvalue, int nreturnTypeId, int nreturnMetaTypeId, QVarLengthArray<int> nvariantargs) :
                object(nobject), methodindex(nmethodindex), hasreturnvalue(nhasreturnvalue), returnTypeId(nreturnTypeId), returnMetaTypeId(nreturnMetaTypeId), varianttypes(nvariantargs)
        {
            
        }
        QObject* object;
        int methodindex;
        bool hasreturnvalue;
        int returnTypeId;
        int returnMetaTypeId;
        QVarLengthArray<int> varianttypes;
        static VALUE s_rccObject;
    };
    VALUE RubyCallCachePrivate::s_rccObject = 0;
    RubyCallCache::RubyCallCache(QObject* object, int methodindex, bool hasreturnvalue, int returnTypeId, int returnMetaTypeId, QVarLengthArray<int> variantargs) :
            d(new RubyCallCachePrivate(object, methodindex, hasreturnvalue, returnTypeId, returnMetaTypeId, variantargs)), m_self(0)
    {
    }
    RubyCallCache::~RubyCallCache()
    {
        delete d;
    }
    
    QVariant RubyCallCache::execfunction( int argc, VALUE *argv )
    {
        QVariant result;
        int typelistcount = d->varianttypes.count();
        QVarLengthArray<MetaType*> variantargs( typelistcount );
        QVarLengthArray<void*> voidstarargs( typelistcount );

        // set the return value
        if(d->hasreturnvalue) {
            MetaType* returntype;
            if(d->returnTypeId != QVariant::Invalid) {
                returntype = new MetaTypeVariant< QVariant >( QVariant( (QVariant::Type) d->returnTypeId ) );
            }
            else {
                if(d->returnMetaTypeId == QMetaType::Void) {
                    returntype = new MetaTypeVariant< QVariant >( QVariant() );
                }
                else {
                    //if (id != -1) {
                    void* myClassPtr = QMetaType::construct(d->returnMetaTypeId, 0);
                    //QMetaType::destroy(id, myClassPtr);
                    returntype = new MetaTypeVoidStar( d->returnMetaTypeId, myClassPtr );
                }
            }

            variantargs[0] = returntype;
            voidstarargs[0] = returntype->toVoidStar();
        }
        else {
            variantargs[0] = 0;
            voidstarargs[0] = (void*)0;
        }
        
                //Set the arguments values
        
        for(int idx = 1; idx < typelistcount; ++idx) {
            MetaType* metatype = RubyMetaTypeFactory::create( d->varianttypes[idx ], argv[idx]);
            if(! metatype) {
                // Seems RubyMetaTypeFactory::create returned an invalid RubyType.
                krosswarning( QString("RubyExtension::callMetaMethod Aborting cause RubyMetaTypeFactory::create returned NULL.") );
                for(int i = 0; i < idx; ++i) // Clear already allocated instances.
                    delete variantargs[i];
                return QVariant(false); // abort execution.
            }
            variantargs[idx] = metatype;
            voidstarargs[idx] = metatype->toVoidStar();
        }

        
                // call the method now
        int r = d->object->qt_metacall(QMetaObject::InvokeMetaMethod, d->methodindex, &voidstarargs[0]);
        #ifdef KROSS_RUBY_EXTENSION_DEBUG
            krossdebug( QString("RESULT nr=%1").arg(r) );
        #else
            Q_UNUSED(r);
        #endif

                // eval the return-value
        if(d->hasreturnvalue) {
            int tp = d->returnTypeId;
            if(tp == QVariant::UserType /*|| tp == QVariant::Invalid*/) {
                tp = d->returnMetaTypeId;
                //QObject* obj = (*reinterpret_cast< QObject*(*)>( variantargs[0]->toVoidStar() ));
            }
            result = QVariant(tp, variantargs[0]->toVoidStar());
            #ifdef KROSS_RUBY_EXTENSION_DEBUG
                krossdebug( QString("Returnvalue id=%1 metamethod.typename=%2 variant.toString=%3 variant.typeName=%4").arg(tp).arg(metamethod.typeName()).arg(result.toString()).arg(result.typeName()) );
            #endif
        }
        for(int idx = 1; idx < typelistcount; ++idx)
        {
            delete variantargs[idx];
        }
        return result;
    }
    void RubyCallCache::delete_object(void* object)
    {
        #ifdef KROSS_RUBY_EXTENSION_DEBUG
            krossdebug("RubyCallCache::delete_object");
        #endif
        RubyCallCache* extension = static_cast< RubyCallCache* >(object);
        delete extension;
        extension = 0;
    }
    VALUE RubyCallCache::method_cacheexec(int argc, VALUE *argv, VALUE self)
    {
        #ifdef KROSS_RUBY_EXTENSION_DEBUG
            krossdebug("RubyCallCache::method_cacheexec");
        #endif
        RubyCallCache* callcache;
        Data_Get_Struct(self, RubyCallCache, callcache);
        QVariant result = callcache->execfunction(argc, argv);
        return result.isNull() ? 0 : RubyType<QVariant>::toVALUE(result);
    }
    VALUE RubyCallCache::toValue()
    {
        if(m_self == 0)
        {
            if(RubyCallCachePrivate::s_rccObject  == 0)
            {
                RubyCallCachePrivate::s_rccObject = rb_define_class("KrossCallCache", rb_cObject);
                rb_define_method(RubyCallCachePrivate::s_rccObject, "cacheexec",  (VALUE (*)(...))RubyCallCache::method_cacheexec, -1);
            }
            m_self = Data_Wrap_Struct(RubyCallCachePrivate::s_rccObject, 0, RubyCallCache::delete_object, this);
        }
        return m_self;
    }
}
