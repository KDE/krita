/*
 *  This file is part of KimageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_progress_updater.h"

KisProgressUpdater::KisProgressUpdater(KisProgressInterface* progressInterface, KoProgressProxy* proxy, KoProgressUpdater::Mode mode)
    : KoProgressUpdater(proxy, mode)
    , m_interface(progressInterface)
{
    Q_ASSERT(progressInterface);
    Q_ASSERT(proxy);
    m_interface->attachUpdater(this);
}

KisProgressUpdater::~KisProgressUpdater()
{
    m_interface->detachUpdater(this);
}
