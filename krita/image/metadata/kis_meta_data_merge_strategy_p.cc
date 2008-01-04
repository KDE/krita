/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_meta_data_merge_strategy_p.h"

#include <klocale.h>

#include "kis_debug.h"

#include "kis_meta_data_entry.h"
#include "kis_meta_data_store.h"
#include "kis_meta_data_value.h"

using namespace KisMetaData;

//-------------------------------------------//
//------------ DropMergeStrategy ------------//
//-------------------------------------------//

DropMergeStrategy::DropMergeStrategy()
{
}

DropMergeStrategy::~DropMergeStrategy()
{
}

QString DropMergeStrategy::id() const
{
    return "Drop";
}
QString DropMergeStrategy::name() const
{
    return i18n("Drop");
}

QString DropMergeStrategy::description() const
{
    return i18n("Drop all meta data");
}

void DropMergeStrategy::merge(Store* dst, QList<const Store*> srcs) const
{
    Q_UNUSED(dst);
    Q_UNUSED(srcs);
    dbgImage << "Drop meta data";
}

//---------------------------------------//
//---------- DropMergeStrategy ----------//
//---------------------------------------//

PriorityToFirstMergeStrategy::PriorityToFirstMergeStrategy()
{
}

PriorityToFirstMergeStrategy::~PriorityToFirstMergeStrategy()
{
}

QString PriorityToFirstMergeStrategy::id() const
{
    return "PriorityToFirst";
}
QString PriorityToFirstMergeStrategy::name() const
{
    return i18n("Priority to first meta data");
}

QString PriorityToFirstMergeStrategy::description() const
{
    return i18n("Use in priority the meta data from the layers at the bottom of the stack.");
}

void PriorityToFirstMergeStrategy::merge(Store* dst, QList<const Store*> srcs) const
{
    dbgImage << "Priority to first meta data";
    
    foreach( const Store* store, srcs )
    {
        QList<QString> keys = store->keys();
        foreach(QString key, keys)
        {
            if( not dst->hasEntry( key ) )
            {
                dst->addEntry( store->getEntry( key ) );
            }
        }
    }
}




//-------------------------------------------//
//------ OnlyIdenticalMergeStrategy ---------//
//-------------------------------------------//

OnlyIdenticalMergeStrategy::OnlyIdenticalMergeStrategy()
{
}

OnlyIdenticalMergeStrategy::~OnlyIdenticalMergeStrategy()
{
}

QString OnlyIdenticalMergeStrategy::id() const
{
    return "OnlyIdentical";
}
QString OnlyIdenticalMergeStrategy::name() const
{
    return i18n("Only identical");
}

QString OnlyIdenticalMergeStrategy::description() const
{
    return i18n("Keep only meta data that are identical");
}

void OnlyIdenticalMergeStrategy::merge(Store* dst, QList<const Store*> srcs) const
{
    dbgImage << "OnlyIdenticalMergeStrategy";
    dbgImage << "Priority to first meta data";
    
    Q_ASSERT(srcs.size() > 0 );
    QList<QString> keys = srcs[0]->keys();
    foreach(QString key, keys)
    {
        bool keep = true;
        const Value& v = srcs[0]->getEntry(key).value();
        foreach( const Store* store, srcs )
        {
            if( not( store->hasEntry( key ) and store->getEntry( key ).value() == v ) )
            {
                keep = false;
                break;
            }
        }
        if(keep)
        {
            dst->addEntry( srcs[0]->getEntry( key ) );
        }
    }
}
