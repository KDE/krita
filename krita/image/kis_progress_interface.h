/*
 *  This file is part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_PROGRESS_INTERFACE_H
#define KIS_PROGRESS_INTERFACE_H

#include <KoProgressUpdater.h>
#include <krita_export.h>

class KRITAIMAGE_EXPORT KisProgressInterface
{
public:

    virtual ~KoProgressInterface();
    virtual KoProgressUpdater* createUpdater() = 0;
    virtual void detachUpdater(KoProgressUpdater* updater) = 0;
    virtual void attachUpdater(KoProgressUpdater* updater) = 0;
};


#endif // KIS_PROGRESS_INTERFACE_H
