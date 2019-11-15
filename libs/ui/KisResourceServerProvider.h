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

#include <brushengine/kis_paintop_preset.h>

#include "kritaui_export.h"
#include "KisWindowLayoutResource.h"

class KisWorkspaceResource;
class KisSessionResource;
class KisPSDLayerStyle;

typedef KoResourceServer<KisPaintOpPreset> KisPaintOpPresetResourceServer;

class KRITAUI_EXPORT KisResourceServerProvider : public QObject
{
    Q_OBJECT

public:
    KisResourceServerProvider();
    ~KisResourceServerProvider() override;

    static KisResourceServerProvider* instance();

    KisPaintOpPresetResourceServer* paintOpPresetServer();
    KoResourceServer<KisWorkspaceResource>* workspaceServer();
    KoResourceServer<KisWindowLayoutResource>* windowLayoutServer();
    KoResourceServer<KisSessionResource>* sessionServer();
    KoResourceServer<KisPSDLayerStyle>* layerStyleServer();

private:

    KisResourceServerProvider(const KisResourceServerProvider&);
    KisResourceServerProvider operator=(const KisResourceServerProvider&);

    KisPaintOpPresetResourceServer *m_paintOpPresetServer;
    KoResourceServer<KisWorkspaceResource> *m_workspaceServer;
    KoResourceServer<KisWindowLayoutResource> *m_windowLayoutServer;
    KoResourceServer<KisSessionResource> *m_sessionServer;
    KoResourceServer<KisPSDLayerStyle> *m_layerStyleServer;
};

#endif // KIS_RESOURCESERVERPROVIDER_H_
