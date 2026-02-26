/*
 * SPDX-FileCopyrightText: 2025 Krita Project
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "ComfyUIRemotePlugin.h"
#include <kpluginfactory.h>
#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>
#include "ComfyUIRemoteDock.h"

K_PLUGIN_FACTORY_WITH_JSON(ComfyUIRemotePluginFactory, "kritacomfyuiremote.json", registerPlugin<ComfyUIRemotePlugin>();)

class ComfyUIRemoteDockFactory : public KoDockFactoryBase
{
public:
    QString id() const override {
        return QString("ComfyUIRemote");
    }

    Qt::DockWidgetArea defaultDockWidgetArea() const override {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget *createDockWidget() override {
        ComfyUIRemoteDock *dock = new ComfyUIRemoteDock();
        dock->setObjectName(id());
        return dock;
    }

    DockPosition defaultDockPosition() const override {
        return DockMinimized;
    }
};

ComfyUIRemotePlugin::ComfyUIRemotePlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new ComfyUIRemoteDockFactory());
}

ComfyUIRemotePlugin::~ComfyUIRemotePlugin()
{
}

#include "ComfyUIRemotePlugin.moc"
