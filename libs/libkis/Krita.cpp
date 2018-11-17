/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "Krita.h"

#include <QPointer>
#include <QVariant>
#include <QStringList>

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoDockRegistry.h>
#include <KoColorSpaceEngine.h>
#include <KoColorModelStandardIds.h>
#include <KoID.h>

#include <kis_filter_strategy.h>
#include <kactioncollection.h>
#include <KisPart.h>
#include <KisMainWindow.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_action.h>
#include <KisViewManager.h>
#include <KritaVersionWrapper.h>
#include <kis_filter_registry.h>
#include <kis_filter.h>
#include <kis_filter_configuration.h>
#include <kis_properties_configuration.h>
#include <kis_config.h>
#include <KisResourceServerProvider.h>
#include <kis_workspace_resource.h>
#include <brushengine/kis_paintop_preset.h>
#include <kis_brush_server.h>
#include <KoResourceServerProvider.h>
#include <kis_action_registry.h>
#include <kis_icon_utils.h>

#include "View.h"
#include "Document.h"
#include "Window.h"
#include "Extension.h"
#include "DockWidgetFactoryBase.h"
#include "Filter.h"
#include "InfoObject.h"
#include "Resource.h"

Krita* Krita::s_instance = 0;

struct Krita::Private {
    Private() {}
    QList<Extension*> extensions;
    bool batchMode {false};
    Notifier *notifier{new Notifier()};
};

Krita::Krita(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    qRegisterMetaType<Notifier*>();
    connect(KisPart::instance(), SIGNAL(sigWindowAdded(KisMainWindow*)), SLOT(mainWindowAdded(KisMainWindow*)));
}

Krita::~Krita()
{
    qDeleteAll(d->extensions);
    delete d->notifier;
    delete d;
}

QList<QAction *> Krita::actions() const
{
    KisMainWindow *mainWindow = KisPart::instance()->currentMainwindow();
    if (!mainWindow) {
        return QList<QAction*>();
    }
    KActionCollection *actionCollection = mainWindow->actionCollection();
    return actionCollection->actions();
}

QAction *Krita::action(const QString &name) const
{
    KisMainWindow *mainWindow = KisPart::instance()->currentMainwindow();
    if (!mainWindow) {
        return 0;
    }
    KActionCollection *actionCollection = mainWindow->actionCollection();
    QAction *action = actionCollection->action(name);
    return action;
}

Document* Krita::activeDocument() const
{
    KisMainWindow *mainWindow = KisPart::instance()->currentMainwindow();
    if (!mainWindow) {
        return 0;
    }
    KisView *view = mainWindow->activeView();
    if (!view) {
        return 0;
    }
    KisDocument *document = view->document();
    return new Document(document);
}

void Krita::setActiveDocument(Document* value)
{
    Q_FOREACH(KisView *view, KisPart::instance()->views()) {
        if (view->document() == value->document().data()) {
            view->activateWindow();
            break;
        }
    }
}

bool Krita::batchmode() const
{
    return d->batchMode;
}

void Krita::setBatchmode(bool value)
{
    d->batchMode = value;
}


QList<Document *> Krita::documents() const
{
    QList<Document *> ret;
    foreach(QPointer<KisDocument> doc, KisPart::instance()->documents()) {
        ret << new Document(doc);
    }
    return ret;
}

QStringList Krita::filters() const
{
    QStringList ls = KisFilterRegistry::instance()->keys();
    std::sort(ls.begin(), ls.end());
    return ls;
}

Filter *Krita::filter(const QString &name) const
{
    if (!filters().contains(name)) return 0;

    Filter *filter = new Filter();
    filter->setName(name);
    KisFilterSP f = KisFilterRegistry::instance()->value(name);
    KisFilterConfigurationSP fc = f->defaultConfiguration();
    InfoObject *info = new InfoObject(fc);
    filter->setConfiguration(info);
    return filter;
}

QStringList Krita::colorModels() const
{
    QSet<QString> colorModelsIds;
    QList<KoID> ids = KoColorSpaceRegistry::instance()->colorModelsList(KoColorSpaceRegistry::AllColorSpaces);
    Q_FOREACH(KoID id, ids) {
        colorModelsIds << id.id();
    }
    return colorModelsIds.toList();
}

QStringList Krita::colorDepths(const QString &colorModel) const
{
    QSet<QString> colorDepthsIds;
    QList<KoID> ids = KoColorSpaceRegistry::instance()->colorDepthList(colorModel, KoColorSpaceRegistry::AllColorSpaces);
    Q_FOREACH(KoID id, ids) {
        colorDepthsIds << id.id();
    }
    return colorDepthsIds.toList();
}

QStringList Krita::filterStrategies() const
{
    return KisFilterStrategyRegistry::instance()->keys();
}

QStringList Krita::profiles(const QString &colorModel, const QString &colorDepth) const
{
    QSet<QString> profileNames;
    QString id = KoColorSpaceRegistry::instance()->colorSpaceId(colorModel, colorDepth);
    QList<const KoColorProfile *> profiles = KoColorSpaceRegistry::instance()->profilesFor(id);
    Q_FOREACH(const KoColorProfile *profile, profiles) {
        profileNames << profile->name();
    }
    QStringList r = profileNames.toList();
    r.sort();
    return r;
}

bool Krita::addProfile(const QString &profilePath)
{
    KoColorSpaceEngine *iccEngine = KoColorSpaceEngineRegistry::instance()->get("icc");
    return iccEngine->addProfile(profilePath);
}

Notifier* Krita::notifier() const
{
    return d->notifier;
}

QString Krita::version() const
{
    return KritaVersionWrapper::versionString(true);
}

QList<View *> Krita::views() const
{
    QList<View *> ret;
    foreach(QPointer<KisView> view, KisPart::instance()->views()) {
        ret << new View(view);
    }
    return ret;
}

Window *Krita::activeWindow() const
{
    KisMainWindow *mainWindow = KisPart::instance()->currentMainwindow();
    if (!mainWindow) {
        return 0;
    }
    return new Window(mainWindow);
}

QList<Window*>  Krita::windows() const
{
    QList<Window*> ret;
    foreach(QPointer<KisMainWindow> mainWin, KisPart::instance()->mainWindows()) {
        ret << new Window(mainWin);
    }
    return ret;
}

QMap<QString, Resource *> Krita::resources(const QString &type) const
{
    QMap<QString, Resource *> resources = QMap<QString, Resource *> ();

    if (type.toLower() == "pattern") {
        KoResourceServer<KoPattern>* server = KoResourceServerProvider::instance()->patternServer();
        Q_FOREACH (KoResourceSP res, server->resources()) {
            resources[res->name()] = new Resource(res);
        }
    }
    else if (type.toLower() == "gradient") {
        KoResourceServer<KoAbstractGradient>* server = KoResourceServerProvider::instance()->gradientServer();
        Q_FOREACH (KoResourceSP res, server->resources()) {
            resources[res->name()] = new Resource(res);
        }
    }
    else if (type.toLower() == "brush") {
        KisBrushResourceServer* server = KisBrushServer::instance()->brushServer();
        Q_FOREACH (KisBrushSP res, server->resources()) {
            resources[res->name()] = new Resource(res);
        }
    }
    else if (type.toLower() == "preset") {
        KisPaintOpPresetResourceServer* server = KisResourceServerProvider::instance()->paintOpPresetServer();
        Q_FOREACH (KisPaintOpPresetSP res, server->resources()) {
            resources[res->name()] = new Resource(res);
        }
    }
    else if (type.toLower() == "palette") {
        KoResourceServer<KoColorSet>* server = KoResourceServerProvider::instance()->paletteServer();
        Q_FOREACH (KoResourceSP res, server->resources()) {
            resources[res->name()] = new Resource(res);
        }
    }
    else if (type.toLower() == "workspace") {
        KoResourceServer< KisWorkspaceResource >* server = KisResourceServerProvider::instance()->workspaceServer();
        Q_FOREACH (KoResourceSP res, server->resources()) {
            resources[res->name()] = new Resource(res);
        }
    }
    return resources;
}

QStringList Krita::recentDocuments() const
{
    KConfigGroup grp = KSharedConfig::openConfig()->group(QString("RecentFiles"));
    QStringList keys = grp.keyList();
    QStringList recentDocuments;

    for(int i = 0; i <= keys.filter("File").count(); i++)
        recentDocuments << grp.readEntry(QString("File%1").arg(i), QString(""));

    return recentDocuments;
}

Document* Krita::createDocument(int width, int height, const QString &name, const QString &colorModel, const QString &colorDepth, const QString &profile, double resolution)
{
    KisDocument *document = KisPart::instance()->createDocument();
    KisPart::instance()->addDocument(document);
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->colorSpace(colorModel, colorDepth, profile);
    Q_ASSERT(cs);

    QColor qc(Qt::white);
    qc.setAlpha(0);
    KoColor bgColor(qc, cs);

    if (!document->newImage(name, width, height, cs, bgColor, true, 1, "", double(resolution / 72) )) {
        return 0;
    }

    Q_ASSERT(document->image());
    return new Document(document);
}

Document* Krita::openDocument(const QString &filename)
{
    KisDocument *document = KisPart::instance()->createDocument();
    document->setFileBatchMode(this->batchmode());
    KisPart::instance()->addDocument(document);
    document->openUrl(QUrl::fromLocalFile(filename), KisDocument::DontAddToRecent);
    document->setFileBatchMode(false);
    return new Document(document);
}

Window* Krita::openWindow()
{
    KisMainWindow *mw = KisPart::instance()->createMainWindow();
    return new Window(mw);
}

void Krita::addExtension(Extension* extension)
{
    d->extensions.append(extension);
}

QList< Extension* > Krita::extensions()
{
    return d->extensions;
}

void Krita::writeSetting(const QString &group, const QString &name, const QString &value)
{
    KConfigGroup grp = KSharedConfig::openConfig()->group(group);
    grp.writeEntry(name, value);
}

QString Krita::readSetting(const QString &group, const QString &name, const QString &defaultValue)
{
    KConfigGroup grp = KSharedConfig::openConfig()->group(group);
    return grp.readEntry(name, defaultValue);
}

QIcon Krita::icon(QString &iconName) const
{
    return KisIconUtils::loadIcon(iconName);
}

void Krita::addDockWidgetFactory(DockWidgetFactoryBase* factory)
{
    KoDockRegistry::instance()->add(factory);
}

Krita* Krita::instance()
{
    if (!s_instance)
    {
        s_instance = new Krita;
    }
    return s_instance;
}

/**
 * Scripter.fromVariant(variant)
 * variant is a QVariant
 * returns instance of QObject-subclass
 *
 * This is a helper method for PyQt because PyQt cannot cast a variant to a QObject or QWidget
 */
QObject *Krita::fromVariant(const QVariant& v)
{

    if (v.canConvert< QWidget* >())
    {
        QObject* obj = qvariant_cast< QWidget* >(v);
        return obj;
    }
    else if (v.canConvert< QObject* >())
    {
        QObject* obj = qvariant_cast< QObject* >(v);
        return obj;
    }
    else
        return 0;
}

QString Krita::krita_i18n(const QString &text)
{
    return i18n(text.toUtf8().constData());
}

void Krita::mainWindowAdded(KisMainWindow *kisWindow)
{
    Q_FOREACH(Extension *extension, d->extensions) {
        Window window(kisWindow);
        extension->createActions(&window);
    }
}
