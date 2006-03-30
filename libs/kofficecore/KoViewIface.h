/* This file is part of the KDE project
   Copyright (c) 2000 Simon Hausmann <hausmann@kde.org>

   $Id$

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#ifndef __KoViewIface_h__
#define __KoViewIface_h__

#include <qmap.h>

#include <dcopobject.h>
#include <dcopref.h>

#include "koffice_export.h"
//Added by qt3to4:

class KDCOPActionProxy;
class KoView;

class KOFFICECORE_EXPORT KoViewIface : public DCOPObject
{
    K_DCOP
public:
    KoViewIface( KoView *view );
    KoViewIface( const char *name, KoView *view );
    // TODO same args order as KoDocumentIface

    virtual ~KoViewIface();

    // Generate a name for this interface. Automatically used if
    // the first constructor is used.
    static DCOPCString newIfaceName();

k_dcop:
    DCOPRef action( const DCOPCString &name );
    DCOPCStringList actions();
    QMap<DCOPCString,DCOPRef> actionMap();

protected:
    KoView *m_pView;
    KDCOPActionProxy *m_actionProxy;
};

#endif
