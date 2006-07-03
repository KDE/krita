/* This file is part of the KDE project
   Copyright (c) 2000 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2006 Fredrik Edemar <f_edemar@linux.se>

   $Id: KoViewIface.h 539508 2006-05-10 20:01:40Z mlaurent $

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
#ifndef __KoViewAdaptor_h__
#define __KoViewAdaptor_h__

#include <QMap>
#include <QObject>
#include <QtDBus/QtDBus>

class QString;
class QStringList;

#include "KoViewAdaptor.h"
#include "koffice_export.h"
//Added by qt3to4:

// class KDCOPActionProxy;
class KoView;

class KOFFICECORE_EXPORT KoViewAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.koffice.view")
public:
    KoViewAdaptor( KoView *view );
    //KoViewAdaptor( const char *name, KoView *view );
    // TODO same args order as KoDocumentIface

    virtual ~KoViewAdaptor();

public Q_SLOTS: // METHODS
    //DCOPRef action( const DCOPCString &name );
    Q_SCRIPTABLE QStringList/*DCOPCStringList*/ actions();
    //QMap<DCOPCString,DCOPRef> actionMap();

protected:
    KoView *m_pView;
    //KDCOPActionProxy *m_actionProxy;

};

#endif
