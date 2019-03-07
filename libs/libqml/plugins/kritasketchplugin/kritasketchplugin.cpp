/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "kritasketchplugin.h"

#include "ColorSelectorItem.h"
#include "CurveEditorItem.h"
#include "DocumentListModel.h"
#include "ColorImageProvider.h"
#include "FiltersCategoryModel.h"
#include "LayerModel.h"
#include "LayerCompositeDetails.h"
#include "PaletteColorsModel.h"
#include "RecentImagesModel.h"
#include "PresetModel.h"
#include "PresetImageProvider.h"
#include "RecentImageImageProvider.h"
#include "RecentFileManager.h"
#include "IconImageProvider.h"
#include "KisMultiFeedRSSModel.h"
#include "FileSystemModel.h"
#include "CompositeOpModel.h"
#include "KeyboardModel.h"
#include "ColorModelModel.h"
#include "ColorDepthModel.h"
#include "ColorProfileModel.h"
#include <TemplatesModel.h>
#include "Theme.h"


#include "Constants.h"
#include "Settings.h"
#include "SimpleTouchArea.h"
#include "ToolManager.h"
#include "KritaNamespace.h"
#include "PanelConfiguration.h"
#include "DocumentManager.h"
#include "kis_clipboard.h"
#include "KisSketchView.h"

#include <QQmlEngine>
#include <QQmlContext>


static QObject *provideConstantsObject(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return new Constants;
}

static QObject *provideKritaNamespaceObject(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return new KritaNamespace;
}

static QObject *provideKritaRssModelObject(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    MultiFeedRssModel *rssModel = new MultiFeedRssModel;
    rssModel->addFeed(QLatin1String("https://krita.org/en/feed/"));

    return rssModel;
}


void KritaSketchPlugin::registerTypes(const char* uri)
{
    Q_UNUSED(uri)
    Q_ASSERT(uri == QLatin1String("org.krita.sketch"));
    qmlRegisterType<SimpleTouchArea>("org.krita.sketch", 1, 0, "SimpleTouchArea");
    qmlRegisterType<ColorSelectorItem>("org.krita.sketch", 1, 0, "ColorSelectorItem");
    qmlRegisterType<CurveEditorItem>("org.krita.sketch", 1, 0, "CurveEditorItem");
    qmlRegisterType<DocumentListModel>("org.krita.sketch", 1, 0, "DocumentListModel");
    qmlRegisterType<PaletteColorsModel>("org.krita.sketch", 1, 0, "PaletteColorsModel");
    qmlRegisterType<PresetModel>("org.krita.sketch", 1, 0, "PresetModel");
    qmlRegisterType<KisSketchView>("org.krita.sketch", 1, 0, "SketchView");
    qmlRegisterType<LayerModel>("org.krita.sketch", 1, 0, "LayerModel");
    qmlRegisterType<FiltersCategoryModel>("org.krita.sketch", 1, 0, "FiltersCategoryModel");
    qmlRegisterType<RecentImagesModel>("org.krita.sketch", 1, 0, "RecentImagesModel");
    qmlRegisterType<FileSystemModel>("org.krita.sketch", 1, 0, "FileSystemModel");
    qmlRegisterType<ToolManager>("org.krita.sketch", 1, 0, "ToolManager");
    qmlRegisterType<CompositeOpModel>("org.krita.sketch", 1, 0, "CompositeOpModel");
    qmlRegisterType<PanelConfiguration>("org.krita.sketch", 1, 0, "PanelConfiguration");
    qmlRegisterType<KeyboardModel>("org.krita.sketch", 1, 0, "KeyboardModel");
    qmlRegisterType<ColorModelModel>("org.krita.sketch", 1, 0, "ColorModelModel");
    qmlRegisterType<ColorDepthModel>("org.krita.sketch", 1, 0, "ColorDepthModel");
    qmlRegisterType<ColorProfileModel>("org.krita.sketch", 1, 0, "ColorProfileModel");
    qmlRegisterType<Theme>("org.krita.sketch", 1, 0, "Theme");
    qmlRegisterType<TemplatesModel>("org.krita.sketch", 1, 0, "TemplatesModel");

    qmlRegisterSingletonType<Constants>("org.krita.sketch", 1, 0, "Constants", provideConstantsObject);
    qmlRegisterSingletonType<KritaNamespace>("org.krita.sketch", 1, 0, "Krita", provideKritaNamespaceObject);
    qmlRegisterSingletonType<MultiFeedRssModel>("org.krita.sketch", 1, 0, "KritaFeedRssModel", provideKritaRssModelObject);

    qmlRegisterUncreatableType<LayerCompositeDetails>("org.krita.sketch", 1, 0, "LayerCompositeDetails", "This type is returned by the LayerModel class");
}

void KritaSketchPlugin::initializeEngine(QQmlEngine* engine, const char* uri)
{
    Q_UNUSED(uri)
    Q_ASSERT(uri == QLatin1String("org.krita.sketch"));

    engine->addImageProvider(QLatin1String("presetthumb"), new PresetImageProvider);
    engine->addImageProvider(QLatin1String("color"), new ColorImageProvider);
    engine->addImageProvider(QLatin1String("recentimage"), new RecentImageImageProvider);
    engine->addImageProvider(QLatin1String("icon"), new IconImageProvider);

    RecentFileManager *recentFileManager = DocumentManager::instance()->recentFileManager();
    engine->rootContext()->setContextProperty("RecentFileManager", recentFileManager);
    engine->rootContext()->setContextProperty("KisClipBoard", KisClipboard::instance());
    engine->rootContext()->setContextProperty("QMLEngine", engine);
    // This would be a problem, but doesn't seem to be used...
//    engine->rootContext()->setContextProperty("View", d->view);
}

