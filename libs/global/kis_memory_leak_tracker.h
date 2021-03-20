/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _KIS_MEMORY_LEAK_TRACKER_H_
#define _KIS_MEMORY_LEAK_TRACKER_H_

#include <QtGlobal>

#include <kritaglobal_export.h>

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
class KRITAGLOBAL_EXPORT KisMemoryLeakTracker
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
