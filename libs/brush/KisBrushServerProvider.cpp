/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisBrushServerProvider.h"

#include <QDir>
#include <QApplication>

#include <QGlobalStatic>
#include <KoResourcePaths.h>

#include <KoResource.h>

#include <kis_debug.h>

Q_GLOBAL_STATIC(KisBrushServerProvider, s_instance)


KisBrushServerProvider::KisBrushServerProvider()
{
    m_brushServer = new KoResourceServer<KisBrush>(ResourceType::Brushes);
}

KisBrushServerProvider::~KisBrushServerProvider()
{
    delete m_brushServer;
}

KisBrushServerProvider* KisBrushServerProvider::instance()
{
    return s_instance;
}

KoResourceServer<KisBrush>* KisBrushServerProvider::brushServer()
{
    return m_brushServer;
}
