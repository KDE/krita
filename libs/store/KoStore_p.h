// -*- c-basic-offset: 2 -*-
/* This file is part of the KDE project
   Copyright 2004 Nicolas GOUTTE <goutte@kde.org>
   
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

#ifndef __koStore_p_h_
#define __koStore_p_h_


#include <QString>

#include <kurl.h>

class QWidget;

class KoStorePrivate
{
public:
    KoStorePrivate() : fileMode(Local), window(0) {}

    enum FileMode { /*Bad=0,*/ Local = 1, RemoteRead, RemoteWrite };

    /**
     * original URL of the remote file
     * (undefined for a local file)
     */
    KUrl url;
    FileMode fileMode;
    QString localFileName;
    QWidget* window;
};

#endif
