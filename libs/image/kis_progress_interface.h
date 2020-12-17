/*
 *  This file is part of KimageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_PROGRESS_INTERFACE_H
#define KIS_PROGRESS_INTERFACE_H

#include <kritaimage_export.h>

class KoProgressUpdater;

class KRITAIMAGE_EXPORT KisProgressInterface
{
public:

    virtual ~KoProgressInterface();
    virtual void detachUpdater(KoProgressUpdater* updater) = 0;
    virtual void attachUpdater(KoProgressUpdater* updater) = 0;
};


#endif // KIS_PROGRESS_INTERFACE_H
