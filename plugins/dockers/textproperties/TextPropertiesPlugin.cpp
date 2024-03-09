/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "TextPropertiesPlugin.h"

#include <kis_debug.h>
#include <kpluginfactory.h>
#include <klocalizedstring.h>
#include <KoDockFactoryBase.h>

#include "kis_config.h"
#include "kis_types.h"
#include "KisViewManager.h"

#include "TextPropertiesDock.h"
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY_WITH_JSON(TextPropertiesPluginFactory, "krita_textproperties.json", registerPlugin<TextPropertiesPlugin>();)

class TextPropertiesDockFactory : public KoDockFactoryBase {
public:
    TextPropertiesDockFactory()
    {
    }

    QString id() const override
    {
        return QString( "TextProperties" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget* createDockWidget() override
    {
        TextPropertiesDock * dockWidget = new TextPropertiesDock();

        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }

};

TextPropertiesPlugin::TextPropertiesPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new TextPropertiesDockFactory());
}

TextPropertiesPlugin::~TextPropertiesPlugin()
{

}
#include "TextPropertiesPlugin.moc"
