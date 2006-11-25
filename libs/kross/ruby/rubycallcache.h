/***************************************************************************
 * rubycallcache.h
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

#ifndef KROSS_RUBYCALLCACHE_H
#define KROSS_RUBYCALLCACHE_H

#include <ruby.h>

#include <QVarLengthArray>
class QObject;
class QVariant;
namespace Kross {
    class MetaType;
    struct RubyCallCachePrivate;
    class RubyCallCache {
        public:
            RubyCallCache(QObject* object, int methodindex, bool hasreturnvalue, int returnTypeId, int returnMetaTypeId, QVarLengthArray<int> variantargs);
            ~RubyCallCache();
            QVariant execfunction( int argc, VALUE *argv );
            VALUE toValue();
            static VALUE method_cacheexec(int argc, VALUE *argv, VALUE self);
            static void delete_object(void* object);
        private:
            RubyCallCachePrivate* d;
            VALUE m_self;
    };
}

#endif
