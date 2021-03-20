/*
 *  kis_resource_server_provider.h - part of KImageShop
 *
 *  SPDX-FileCopyrightText: 1999 Matthias Elter <elter@kde.org>
 *  SPDX-FileCopyrightText: 2003 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2005 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2003-2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
