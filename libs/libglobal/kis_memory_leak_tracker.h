/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_MEMORY_LEAK_TRACKER_H_
#define _KIS_MEMORY_LEAK_TRACKER_H_


#include <kritaimage_export.h>

#include <config-memory-leak-tracker.h>

// Only linux support the memory leak tracker
#ifndef Q_OS_LINUX
#undef HAVE_MEMORY_LEAK_TRACKER
#endif

// Disable the memory leak tracker on release build
#ifdef NDEBUG
#undef HAVE_MEMORY_LEAK_TRACKER
#endif

/**
 * This class tracks what pointer is reference by who. It is used by
 * the smart pointers to detect leaks.
 *
 * Note that the KisMemoryLeakTracker is currently only available on Linux,
 * and translate to NOOP on other platforms. It is also just a debug tool,
 * and should not be used in a production build of krita.
 */
class KRITAIMAGE_EXPORT KisMemoryLeakTracker
{
public:
    KisMemoryLeakTracker();
    ~KisMemoryLeakTracker();
    static KisMemoryLeakTracker* instance();
    void reference(const void* what, const void* bywho, const char* whatName = 0);
    void dereference(const void* what, const void* bywho);
    void dumpReferences();
    void dumpReferences(const void* what);
public:
    template<typename _T_>
    void reference(const _T_* what, const void* bywho);
    template<typename _T_>
    void dereference(const _T_* what, const void* bywho);
private:
    struct Private;
    Private* const d;
};

#include <typeinfo>

template<typename _T_>
void KisMemoryLeakTracker::reference(const _T_* what, const void* bywho)
{
    reference((void*)what, bywho, typeid(what).name());
}

template<typename _T_>
void KisMemoryLeakTracker::dereference(const _T_* what, const void* bywho)
{
    dereference((void*)what, bywho);
}

#endif
