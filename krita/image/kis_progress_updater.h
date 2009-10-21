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
#ifndef KIS_PROGRESS_UPDATER_H
#define KIS_PROGRESS_UPDATER_H

#include <KoProgressUpdater.h>
#include <krita_export.h>

class KRITAIMAGE_EXPORT KisProgressInterface
{
public:

    virtual ~KisProgressInterface() {};
    virtual KoProgressUpdater* createUpdater(KoProgressUpdater::Mode mode = KoProgressUpdater::Threaded) = 0;
    virtual void detachUpdater(KoProgressUpdater* updater) = 0;
    virtual void attachUpdater(KoProgressUpdater* updater) = 0;
};



/**
 * KisProgressUpdater is an updater that disengages itself automatically
 * from the updater widget when done.
 */
class KRITAIMAGE_EXPORT KisProgressUpdater : public KoProgressUpdater
{
public:
    KisProgressUpdater(KisProgressInterface* progressInterface, KoProgressProxy* proxy,
                       KoProgressUpdater::Mode mode = KoProgressUpdater::Threaded);
    virtual ~KisProgressUpdater();
private:
    KisProgressInterface* m_interface;
};



#endif // KIS_PROGRESS_UPDATER_H
