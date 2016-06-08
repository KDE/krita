/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

    // see note in kis_shared.cc
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
