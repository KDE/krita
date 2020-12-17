/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_SHARED_H_
#define _KIS_SHARED_H_

#include <QAtomicInt>
#include "kritaglobal_export.h"

class KRITAGLOBAL_EXPORT KisShared
{
private:
    KisShared(const KisShared& );
    KisShared& operator=(const KisShared& );
protected:
    KisShared();
    ~KisShared();
public:
    int refCount() {
        return _ref;
    }
    bool ref() {
        return _ref.ref();
    }
    bool deref() {
        Q_ASSERT(_ref > 0);
        return _ref.deref();
    }

    // see note in kis_shared.cpp
    QAtomicInt* sharedWeakReference() {
        if(!_sharedWeakReference) {
            _sharedWeakReference = new QAtomicInt();
            _sharedWeakReference->ref();
        }

        return _sharedWeakReference;
    }

private:
    QAtomicInt _ref;
    QAtomicInt *_sharedWeakReference;
};

#endif
