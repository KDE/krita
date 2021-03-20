/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "kis_memory_leak_tracker.h"

#include <QMutex>
#include <QGlobalStatic>

#include "kis_debug.h"

// Those defines are used to ignore classes that are often leaked due to a KisPaintDevice leak
#define IGNORE_MEMENTO_ITEM
#define IGNORE_TILE

Q_GLOBAL_STATIC(KisMemoryLeakTracker, s_instance)

// Common function
KisMemoryLeakTracker* KisMemoryLeakTracker::instance()
{
    return s_instance;
}
#ifdef HAVE_MEMORY_LEAK_TRACKER

#include <QHash>

#ifdef Q_OS_LINUX

struct BacktraceInfo {
    BacktraceInfo() : trace(0), size(0) {
    }
    ~BacktraceInfo() {
        delete[] trace;
    }
    void** trace;
    int size;
};

#define BACKTRACE_SIZE 10

#include <execinfo.h>

#ifdef HAVE_BACKTRACE_SUPPORT
#define MAKE_BACKTRACEINFO \
    BacktraceInfo* info = new BacktraceInfo; \
    info->trace = new void*[BACKTRACE_SIZE]; \
    int n = backtrace(info->trace, BACKTRACE_SIZE); \
    info->size = n;
#else
#define MAKE_BACKTRACEINFO \
    BacktraceInfo* info = 0;
#endif

struct WhatInfo {
    QHash<const void*, BacktraceInfo*> infos;
    QString name;
};

struct KisMemoryLeakTracker::Private {
    QHash<const void*, WhatInfo > whatWhoWhen;
    template<typename _T_>
    void dumpReferencedObjectsAndDelete(QHash<const _T_*, WhatInfo >&, bool _delete);
    QMutex m;
};

template<typename _T_>
void KisMemoryLeakTracker::Private::dumpReferencedObjectsAndDelete(QHash<const _T_*, WhatInfo >& map, bool _delete)
{
    QMutexLocker l(&m);
    for (typename QHash<const _T_*, WhatInfo >::iterator it = map.begin();
            it != map.end(); ++it) {
        qWarning() << "Object " << it.key() << "(" << it.value().name << ") is still referenced by " << it.value().infos.size() << " objects:";
        for (QHash<const void*, BacktraceInfo*>::iterator it2 = it.value().infos.begin();
                it2 != it.value().infos.end(); ++it2) {
            qWarning() << "Referenced by " << it2.key() << " at:";
#ifdef HAVE_BACKTRACE_SUPPORT
            BacktraceInfo* info = it2.value();
            char** strings = backtrace_symbols(info->trace, info->size);
            for (int i = 0; i < info->size; ++i) {
                qWarning() << strings[i];
            }
            if (_delete) {
                delete info;
                it2.value() = 0;
            }
#else
            Q_UNUSED(_delete);
            qWarning() << "Enable backtrace support by running 'cmake -DHAVE_BACKTRACE_SUPPORT=ON'";
#endif
        }
        qWarning() << "=====";
    }
}

KisMemoryLeakTracker::KisMemoryLeakTracker() : d(new Private)
{
}

KisMemoryLeakTracker::~KisMemoryLeakTracker()
{
    if (d->whatWhoWhen.isEmpty()) {
        qInfo() << "No leak detected.";
    } else {
        qWarning() << "****************************************";
        qWarning() << (d->whatWhoWhen.size()) << " leaks have been detected";
        d->dumpReferencedObjectsAndDelete(d->whatWhoWhen, true);
        qWarning() << "****************************************";
#ifndef NDEBUG
        qFatal("Leaks have been detected... fix krita.");
#endif
    }
    delete d;
}

void KisMemoryLeakTracker::reference(const void* what, const void* bywho, const char* whatName)
{
    if(what == 0x0) return;

    QMutexLocker l(&d->m);

    if (whatName == 0 || ( strcmp(whatName, "PK13KisSharedData") != 0
#ifdef IGNORE_MEMENTO_ITEM
                           && strcmp(whatName, "PK14KisMementoItem") != 0
#endif
#ifdef IGNORE_TILE
                           && strcmp(whatName, "PK7KisTile") != 0
#endif
        ) ) {
        MAKE_BACKTRACEINFO
        d->whatWhoWhen[what].infos[bywho] = info;
        if (whatName) {
            d->whatWhoWhen[what].name = whatName;
        }
    }
}

void KisMemoryLeakTracker::dereference(const void* what, const void* bywho)
{
    QMutexLocker l(&d->m);
    if (d->whatWhoWhen.contains(what)) {
        QHash<const void*, BacktraceInfo*>& whoWhen = d->whatWhoWhen[what].infos;
        delete whoWhen[bywho];
        whoWhen.remove(bywho);
        if (whoWhen.isEmpty()) {
            d->whatWhoWhen.remove(what);
        }
    }
}

void KisMemoryLeakTracker::dumpReferences()
{
    qWarning() << "****************************************";
    qWarning() << (d->whatWhoWhen.size()) << " objects are currently referenced";
    d->dumpReferencedObjectsAndDelete(d->whatWhoWhen, false);
    qWarning() << "****************************************";
}

void KisMemoryLeakTracker::dumpReferences(const void* what)
{
    QMutexLocker l(&d->m);
    if (!d->whatWhoWhen.contains(what)) {
        qWarning() << "Object " << what << " is not tracked";
        return;
    }

    WhatInfo& info = d->whatWhoWhen[what];
    qInfo() << "Object " << what << "(" << info.name << ") is still referenced by " << info.infos.size() << " objects:";
    for (QHash<const void*, BacktraceInfo*>::iterator it2 = info.infos.begin();
            it2 != info.infos.end(); ++it2) {
        qInfo() << "Referenced by " << it2.key() << " at:";
#ifdef HAVE_BACKTRACE_SUPPORT
        BacktraceInfo* info = it2.value();
        char** strings = backtrace_symbols(info->trace, info->size);
        for (int i = 0; i < info->size; ++i) {
            qInfo() << strings[i];
        }
#else
            qInfo() << "Enable backtrace support in kis_memory_leak_tracker.cpp";
#endif
    }
    qInfo() << "=====";
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

void KisMemoryLeakTracker::reference(const void* what, const void* bywho, const char* whatName)
{
    Q_UNUSED(what);
    Q_UNUSED(bywho);
    Q_UNUSED(whatName);
}

void KisMemoryLeakTracker::dereference(const void* what, const void* bywho)
{
    Q_UNUSED(what);
    Q_UNUSED(bywho);
}

void KisMemoryLeakTracker::dumpReferences()
{
}

void KisMemoryLeakTracker::dumpReferences(const void* what)
{
    Q_UNUSED(what);
}

#endif
