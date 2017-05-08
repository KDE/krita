/*
 *  kis_resource_server_provider.h - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Sven Langkamp <sven.langkamp@gmail.com>
 *  Copyright (c) 2003-2008 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_RESOURCESERVERPROVIDER_H_
#define KIS_RESOURCESERVERPROVIDER_H_

#include <KoResourceServer.h>
#include <KoResourceServerAdapter.h>

#include <brushengine/kis_paintop_preset.h>

#include "KisResourceBundle.h"
#include "kritaui_export.h"

class KoResourceLoaderThread;
class KisWorkspaceResource;
class KisPSDLayerStyleCollectionResource;

typedef KoResourceServerSimpleConstruction<KisPaintOpPreset, SharedPointerStoragePolicy<KisPaintOpPresetSP> > KisPaintOpPresetResourceServer;
typedef KoResourceServerAdapter<KisPaintOpPreset, SharedPointerStoragePolicy<KisPaintOpPresetSP> > KisPaintOpPresetResourceServerAdapter;

class KRITAUI_EXPORT KisResourceServerProvider : public QObject
{
    Q_OBJECT

public:
    KisResourceServerProvider();
    ~KisResourceServerProvider() override;

    static KisResourceServerProvider* instance();

    KoResourceServer<KisResourceBundle> *resourceBundleServer();
    KisPaintOpPresetResourceServer* paintOpPresetServer(bool block = true);
    KoResourceServer<KisWorkspaceResource>* workspaceServer(bool block = true);
    KoResourceServer<KisPSDLayerStyleCollectionResource>* layerStyleCollectionServer(bool block = true);

    void brushBlacklistCleanup();

Q_SIGNALS:
    void notifyBrushBlacklistCleanup();

private:

    KisResourceServerProvider(const KisResourceServerProvider&);
    KisResourceServerProvider operator=(const KisResourceServerProvider&);

    KisPaintOpPresetResourceServer *m_paintOpPresetServer;
    KoResourceServer<KisWorkspaceResource> *m_workspaceServer;
    KoResourceServer<KisPSDLayerStyleCollectionResource> *m_layerStyleCollectionServer;
    KoResourceServer<KisResourceBundle> *m_resourceBundleServer;

private:

    KoResourceLoaderThread *m_paintOpPresetThread;
    KoResourceLoaderThread *m_workspaceThread;
    KoResourceLoaderThread *m_layerStyleCollectionThread;

};

#endif // KIS_RESOURCESERVERPROVIDER_H_
