/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include "kis_memory_leak_tracker.h"

#include <kglobal.h>
#include "kis_debug.h"

// Only linux support the memory leak tracker
#ifdef Q_OS_LINUX
#define HAVE_MEMORY_LEAK_TRACKER
#endif

// Disable the memory leak tracker on release build
#ifdef NDEBUG
#undef HAVE_MEMORY_LEAK_TRACKER
#endif

// Common function
KisMemoryLeakTracker* KisMemoryLeakTracker::instance()
{
    K_GLOBAL_STATIC(KisMemoryLeakTracker, s_instance);
    return s_instance;
}

#ifdef HAVE_MEMORY_LEAK_TRACKER

#include <QHash>

#ifdef Q_OS_LINUX

struct BacktraceInfo {
    BacktraceInfo() : trace(0), size(0)
    {
    }
    ~BacktraceInfo()
    {
    }
    void** trace;
    int size;
};

#define BACKTRACE_SIZE 10

#include <execinfo.h>

#define MAKE_BACKTRACEINFO \
    BacktraceInfo* info = new BacktraceInfo; \
    info->trace = new void*[BACKTRACE_SIZE]; \
    int n = backtrace(info->trace, BACKTRACE_SIZE); \
    info->size = n;
    

struct KisMemoryLeakTracker::Private {
    QHash<const void*, QHash<const void*, BacktraceInfo*> > whatWhoWhen;
    QHash<const QObject*, QHash<const void*, BacktraceInfo*> > whatQObjWhoWhen;
    template<typename _T_>
    void dumpReferencedObjectsAndDelete( QHash<const _T_*, QHash<const void*, BacktraceInfo*> >& );
};

template<typename _T_>
void KisMemoryLeakTracker::Private::dumpReferencedObjectsAndDelete( QHash<const _T_*, QHash<const void*, BacktraceInfo*> >& map)
{
    for(typename QHash<const _T_*, QHash<const void*, BacktraceInfo*> >::iterator it = map.begin();
        it != map.end(); ++it)
    {
        errKrita << "Object " << it.key() << " is still referenced by " << it.value().size() << " objects:";
        for( QHash<const void*, BacktraceInfo*>::iterator it2 = it.value().begin();
            it2 != it.value().end(); ++it2 )
        {
            BacktraceInfo* info = it2.value();
            char** strings = backtrace_symbols (info->trace, info->size);
            errKrita << "Referenced by " << it2.key() << " at:";
            for(int i = 0; i < info->size; ++i)
            {
                errKrita << strings[i];
            }
            delete info;
            it2.value() = 0;
        }
        errKrita << "=====";
    }
}

KisMemoryLeakTracker::KisMemoryLeakTracker() : d(new Private)
{
}

KisMemoryLeakTracker::~KisMemoryLeakTracker()
{
    if(d->whatQObjWhoWhen.isEmpty() && d->whatWhoWhen.isEmpty())
    {
        dbgKrita << "No leak detected.";
    } else {
        errKrita << "****************************************";
        errKrita << (d->whatQObjWhoWhen.size() + d->whatWhoWhen.size()) << " leaks have been detected";
        d->dumpReferencedObjectsAndDelete(d->whatWhoWhen);
        d->dumpReferencedObjectsAndDelete(d->whatQObjWhoWhen);
        errKrita << "****************************************";
    }
    delete d;
}

void KisMemoryLeakTracker::reference(const void* what, const void* bywho)
{
    MAKE_BACKTRACEINFO
    d->whatWhoWhen[what][bywho] = info;
}

void KisMemoryLeakTracker::dereference(const void* what, const void* bywho)
{
    QHash<const void*, BacktraceInfo*>& whoWhen = d->whatWhoWhen[what];
    delete whoWhen[bywho];
    whoWhen.remove(bywho);
    if (whoWhen.isEmpty())
    {
        d->whatWhoWhen.remove(what);
    }
}

void KisMemoryLeakTracker::reference(const QObject* what, const void* bywho)
{
    MAKE_BACKTRACEINFO
    d->whatQObjWhoWhen[what][bywho] = info;
}

void KisMemoryLeakTracker::dereference(const QObject* what, const void* bywho)
{
    QHash<const void*, BacktraceInfo*>& whoWhen = d->whatQObjWhoWhen[what];
    delete whoWhen[bywho];
    whoWhen.remove(bywho);
    if (whoWhen.isEmpty())
    {
        d->whatQObjWhoWhen.remove(what);
    }
}
#else
#error "Hum, no memory leak tracker for your platform"
#endif

#else

KisMemoryLeakTracker::KisMemoryLeakTracker() : d(0)
{
}

KisMemoryLeakTracker::~KisMemoryLeakTracker()
{
}

void KisMemoryLeakTracker::reference(const void* what, const void* bywho)
{
}

void KisMemoryLeakTracker::dereference(const void* what, const void* bywho)
{
}

void KisMemoryLeakTracker::reference(const QObject* what, const void* bywho)
{
}

void KisMemoryLeakTracker::dereference(const QObject* what, const void* bywho)
{
}

#endif
