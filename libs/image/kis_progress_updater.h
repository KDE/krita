/*
 *  This file is part of KimageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_PROGRESS_UPDATER_H
#define KIS_PROGRESS_UPDATER_H

#include <KoProgressUpdater.h>
#include <kritaimage_export.h>

class KRITAIMAGE_EXPORT KisProgressInterface
{
public:

    virtual ~KisProgressInterface() {};
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
    ~KisProgressUpdater() override;
private:
    KisProgressInterface* m_interface;
};



#endif // KIS_PROGRESS_UPDATER_H
