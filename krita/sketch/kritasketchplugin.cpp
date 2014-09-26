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
#include "KisSketchView.h"
#include "ColorImageProvider.h"
#include "FiltersCategoryModel.h"
#include "LayerModel.h"
#include "LayerCompositeDetails.h"
#include "PaletteColorsModel.h"
#include "RecentImagesModel.h"
#include "PaletteModel.h"
#include "PresetModel.h"
#include "PresetImageProvider.h"
#include "RecentImageImageProvider.h"
#include "RecentFileManager.h"
#include "MultiFeedRSSModel.h"
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
#include "ImageBuilder.h"
#include "KritaNamespace.h"
#include "PanelConfiguration.h"
#include "DocumentManager.h"
#include "kis_doc2.h"
#include "kis_view2.h"
#include "kis_clipboard.h"

#include <QDeclarativeEngine>
#include <QDeclarativeContext>

void KritaSketchPlugin::registerTypes(const char* uri)
{
    Q_UNUSED(uri)
    Q_ASSERT(uri == QLatin1String("org.krita.sketch"));
    qmlRegisterType<SimpleTouchArea>("org.krita.sketch", 1, 0, "SimpleTouchArea");
    qmlRegisterType<ColorSelectorItem>("org.krita.sketch", 1, 0, "ColorSelectorItem");
    qmlRegisterType<CurveEditorItem>("org.krita.sketch", 1, 0, "CurveEditorItem");
    qmlRegisterType<DocumentListModel>("org.krita.sketch", 1, 0, "DocumentListModel");
    qmlRegisterType<PaletteModel>("org.krita.sketch", 1, 0, "PaletteModel");
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

    qmlRegisterUncreatableType<LayerCompositeDetails>("org.krita.sketch", 1, 0, "LayerCompositeDetails", "This type is returned by the LayerModel class");
}

void KritaSketchPlugin::initializeEngine(QDeclarativeEngine* engine, const char* uri)
{
    Q_UNUSED(uri)
    Q_ASSERT(uri == QLatin1String("org.krita.sketch"));

    Constants *constants = new Constants( this );
    Settings *settings = new Settings( this );
    DocumentManager::instance()->setSettingsManager( settings );
    RecentFileManager *recentFileManager = DocumentManager::instance()->recentFileManager();

    engine->addImageProvider(QLatin1String("presetthumb"), new PresetImageProvider);
    engine->addImageProvider(QLatin1String("color"), new ColorImageProvider);
    engine->addImageProvider(QLatin1String("recentimage"), new RecentImageImageProvider);

    KritaNamespace *nameSpace = new KritaNamespace(this);
    engine->rootContext()->setContextProperty("Krita", nameSpace);

    engine->rootContext()->setContextProperty("Constants", constants);
    engine->rootContext()->setContextProperty("Settings", settings);
    engine->rootContext()->setContextProperty("RecentFileManager", recentFileManager);
    engine->rootContext()->setContextProperty("KisClipBoard", KisClipboard::instance());
    engine->rootContext()->setContextProperty("QMLEngine", engine);
    // This would be a problem, but doesn't seem to be used...
//    engine->rootContext()->setContextProperty("View", d->view);

    Welcome::MultiFeedRssModel *rssModel = new Welcome::MultiFeedRssModel(this);
    rssModel->addFeed(QLatin1String("https://krita.org/feed"));
    engine->rootContext()->setContextProperty("aggregatedFeedsModel", rssModel);
}

#include "kritasketchplugin.moc"
