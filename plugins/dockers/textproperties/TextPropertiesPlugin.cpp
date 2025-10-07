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
#include <KisStaticInitializer.h>

#include "kis_config.h"
#include "kis_types.h"
#include "KisViewManager.h"

#include "TextPropertiesDock.h"
#include <KoDockRegistry.h>
#include <QQmlEngine>

#include <text/lager/KoSvgTextPropertiesModel.h>
#include <text/lager/CssLengthPercentageModel.h>
#include <text/lager/LineHeightModel.h>
#include <text/lager/TextIndentModel.h>
#include <text/lager/TabSizeModel.h>
#include <text/lager/TextTransformModel.h>
#include <text/lager/CssFontStyleModel.h>
#include <text/lager/FontVariantLigaturesModel.h>
#include <text/lager/FontVariantNumericModel.h>
#include <text/lager/FontVariantEastAsianModel.h>

#include "FontStyleModel.h"
#include "FontAxesModel.h"
#include "OpenTypeFeatureModel.h"
#include "LocaleHandler.h"
#include "CssQmlUnitConverter.h"
#include "TextPropertyConfigModel.h"
#include "TextPropertiesCanvasObserver.h"

K_PLUGIN_FACTORY_WITH_JSON(TextPropertiesPluginFactory, "krita_textproperties.json", registerPlugin<TextPropertiesPlugin>();)

KIS_DECLARE_STATIC_INITIALIZER {
    qmlRegisterType<TextPropertiesCanvasObserver>("org.krita.flake.text", 1, 0, "TextPropertiesCanvasObserver");
    qmlRegisterType<KoSvgTextPropertiesModel>("org.krita.flake.text", 1, 0, "KoSvgTextPropertiesModel");
    qmlRegisterType<CssLengthPercentageModel>("org.krita.flake.text", 1, 0, "CssLengthPercentageModel");
    qmlRegisterType<LineHeightModel>("org.krita.flake.text", 1, 0, "LineHeightModel");
    qmlRegisterType<TextIndentModel>("org.krita.flake.text", 1, 0, "TextIndentModel");
    qmlRegisterType<TabSizeModel>("org.krita.flake.text", 1, 0, "TabSizeModel");
    qmlRegisterType<TextTransformModel>("org.krita.flake.text", 1, 0, "TextTransformModel");
    qmlRegisterType<CssFontStyleModel>("org.krita.flake.text", 1, 0, "CssFontStyleModel");
    qmlRegisterType<FontVariantLigaturesModel>("org.krita.flake.text", 1, 0, "FontVariantLigaturesModel");
    qmlRegisterType<FontVariantNumericModel>("org.krita.flake.text", 1, 0, "FontVariantNumericModel");
    qmlRegisterType<FontVariantEastAsianModel>("org.krita.flake.text", 1, 0, "FontVariantEastAsianModel");
    qmlRegisterUncreatableMetaObject(KoSvgText::staticMetaObject, "org.krita.flake.text", 1, 0, "KoSvgText", "Error: Namespace with enums");

    qmlRegisterType<FontStyleModel>("org.krita.flake.text", 1, 0, "FontStyleModel");
    qmlRegisterType<FontAxesModel>("org.krita.flake.text", 1, 0, "FontAxesModel");
    qmlRegisterType<OpenTypeFeatureFilterModel>("org.krita.flake.text", 1, 0, "OpenTypeFeatureFilterModel");
    qmlRegisterType<OpenTypeFeatureModel>("org.krita.flake.text", 1, 0, "OpenTypeFeatureModel");
    qmlRegisterType<CssQmlUnitConverter>("org.krita.flake.text", 1, 0, "CssQmlUnitConverter");
    qmlRegisterType<LocaleHandler>("org.krita.flake.text", 1, 0, "LocaleHandler");
    qmlRegisterType<TextPropertyConfigModel>("org.krita.flake.text", 1, 0, "TextPropertyConfigModel");
    qmlRegisterType<TextPropertyConfigFilterModel>("org.krita.flake.text", 1, 0, "TextPropertyConfigFilterModel");
}

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
