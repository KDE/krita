/*
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 * Copyright (C) 2012 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KisApplication.h"

#include <stdlib.h>
#ifdef Q_OS_WIN
#include <windows.h>
#include <tchar.h>
#endif

#ifdef Q_OS_OSX
#include "osx.h"
#endif

#include <QStandardPaths>
#include <QDesktopWidget>
#include <QDir>
#include <QFile>
#include <QLocale>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QSettings>
#include <QStringList>
#include <QStyle>
#include <QStyleFactory>
#include <QSysInfo>
#include <QTimer>
#include <QWidget>
#include <QImageReader>

#include <klocalizedstring.h>
#include <kdesktopfile.h>
#include <kconfig.h>
#include <kconfiggroup.h>

#include <KoColorSpaceRegistry.h>
#include <KoPluginLoader.h>
#include <KoShapeRegistry.h>
#include <KoDpi.h>
#include "KoConfig.h"
#include <KoHashGeneratorProvider.h>
#include <KoResourcePaths.h>
#include <KisMimeDatabase.h>
#include "thememanager.h"
#include "KisPrintJob.h"
#include "KisDocument.h"
#include "KisMainWindow.h"
#include "KisAutoSaveRecoveryDialog.h"
#include "KisPart.h"
#include <kis_icon.h>
#include "kis_md5_generator.h"
#include "kis_splash_screen.h"
#include "kis_config.h"
#include "flake/kis_shape_selection.h"
#include <filter/kis_filter.h>
#include <filter/kis_filter_registry.h>
#include <filter/kis_filter_configuration.h>
#include <generator/kis_generator_registry.h>
#include <generator/kis_generator.h>
#include <brushengine/kis_paintop_registry.h>
#include <metadata/kis_meta_data_io_backend.h>
#include "kisexiv2/kis_exiv2.h"
#include "KisApplicationArguments.h"
#include <kis_debug.h>
#include "kis_action_registry.h"
#include <kis_brush_server.h>
#include <KisResourceServerProvider.h>
#include <KoResourceServerProvider.h>
#include "kis_image_barrier_locker.h"
#include "opengl/kis_opengl.h"
#include "kis_spin_box_unit_manager.h"
#include "kis_document_aware_spin_box_unit_manager.h"
#include "KisViewManager.h"
#include "kis_workspace_resource.h"
#include <KritaVersionWrapper.h>
#include <dialogs/KisSessionManagerDialog.h>

#include <KisResourceCacheDb.h>
#include <KisResourceLocator.h>
#include <KisResourceLoader.h>
#include <KisResourceLoaderRegistry.h>

#include <kis_gbr_brush.h>
#include <kis_png_brush.h>
#include <kis_svg_brush.h>
#include <kis_imagepipe_brush.h>
#include <KoColorSet.h>
#include <KoSegmentGradient.h>
#include <KoStopGradient.h>
#include <KoPattern.h>
#include <kis_workspace_resource.h>
#include <KisSessionResource.h>
#include <resources/KoSvgSymbolCollectionResource.h>

#include "widgets/KisScreenColorPicker.h"
#include "KisDlgInternalColorSelector.h"


namespace {
const QTime appStartTime(QTime::currentTime());
}

class KisApplication::Private
{
public:
    Private() {}
    QPointer<KisSplashScreen> splashScreen;
    KisAutoSaveRecoveryDialog *autosaveDialog {0};
    QPointer<KisMainWindow> mainWindow; // The first mainwindow we create on startup
    bool batchRun {false};
};

class KisApplication::ResetStarting
{
public:
    ResetStarting(KisSplashScreen *splash, int fileCount)
        : m_splash(splash)
        , m_fileCount(fileCount)
    {
    }

    ~ResetStarting()  {

        if (m_splash) {
            m_splash->hide();
        }
    }

    QPointer<KisSplashScreen> m_splash;
    int m_fileCount;
};


KisApplication::KisApplication(const QString &key, int &argc, char **argv)
    : QtSingleApplication(key, argc, argv)
    , d(new Private)
{
#ifdef Q_OS_OSX
    setMouseCoalescingEnabled(false);
#endif

    KisDlgInternalColorSelector::s_screenColorPickerFactory = KisScreenColorPicker::createScreenColorPicker;


    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());

    setApplicationDisplayName("Krita");
    setApplicationName("krita");
    // Note: Qt docs suggest we set this, but if we do, we get resource paths of the form of krita/krita, which is weird.
    //    setOrganizationName("krita");
    setOrganizationDomain("krita.org");

    QString version = KritaVersionWrapper::versionString(true);
    setApplicationVersion(version);
    setWindowIcon(KisIconUtils::loadIcon("calligrakrita"));

    if (qgetenv("KRITA_NO_STYLE_OVERRIDE").isEmpty()) {
        QStringList styles = QStringList() << "breeze" << "fusion" << "plastique";
        if (!styles.contains(style()->objectName().toLower())) {
            Q_FOREACH (const QString & style, styles) {
                if (!setStyle(style)) {
                    qDebug() << "No" << style << "available.";
                }
                else {
                    qDebug() << "Set style" << style;
                    break;
                }
            }
        }
    }
    else {
        qDebug() << "Style override disabled, using" << style()->objectName();
    }

    KisOpenGL::initialize();
}

#if defined(Q_OS_WIN) && defined(ENV32BIT)
typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

LPFN_ISWOW64PROCESS fnIsWow64Process;

BOOL isWow64()
{
    BOOL bIsWow64 = FALSE;

    //IsWow64Process is not available on all supported versions of Windows.
    //Use GetModuleHandle to get a handle to the DLL that contains the function
    //and GetProcAddress to get a pointer to the function if available.

    fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
                GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

    if(0 != fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
        {
            //handle error
        }
    }
    return bIsWow64;
}
#endif

void KisApplication::initializeGlobals(const KisApplicationArguments &args)
{
    int dpiX = args.dpiX();
    int dpiY = args.dpiY();
    if (dpiX > 0 && dpiY > 0) {
        KoDpi::setDPI(dpiX, dpiY);
    }
}

void KisApplication::addResourceTypes()
{
    //    qDebug() << "addResourceTypes();";
    // All Krita's resource types
    KoResourcePaths::addResourceType("kis_pics", "data", "/pics/");
    KoResourcePaths::addResourceType("kis_images", "data", "/images/");
    KoResourcePaths::addResourceType("metadata_schema", "data", "/metadata/schemas/");
    KoResourcePaths::addResourceType("kis_brushes", "data", "/brushes/");
    KoResourcePaths::addResourceType("kis_taskset", "data", "/taskset/");
    KoResourcePaths::addResourceType("kis_taskset", "data", "/taskset/");
    KoResourcePaths::addResourceType("gmic_definitions", "data", "/gmic/");
    KoResourcePaths::addResourceType("kis_resourcebundles", "data", "/bundles/");
    KoResourcePaths::addResourceType("kis_defaultpresets", "data", "/defaultpresets/");
    KoResourcePaths::addResourceType("kis_paintoppresets", "data", "/paintoppresets/");
    KoResourcePaths::addResourceType("kis_workspaces", "data", "/workspaces/");
    KoResourcePaths::addResourceType("kis_windowlayouts", "data", "/windowlayouts/");
    KoResourcePaths::addResourceType("kis_sessions", "data", "/sessions/");
    KoResourcePaths::addResourceType("psd_layer_style_collections", "data", "/asl");
    KoResourcePaths::addResourceType("ko_patterns", "data", "/patterns/", true);
    KoResourcePaths::addResourceType("ko_gradients", "data", "/gradients/");
    KoResourcePaths::addResourceType("ko_gradients", "data", "/gradients/", true);
    KoResourcePaths::addResourceType("ko_palettes", "data", "/palettes/", true);
    KoResourcePaths::addResourceType("kis_shortcuts", "data", "/shortcuts/");
    KoResourcePaths::addResourceType("kis_actions", "data", "/actions");
    KoResourcePaths::addResourceType("icc_profiles", "data", "/color/icc");
    KoResourcePaths::addResourceType("icc_profiles", "data", "/profiles/");
    KoResourcePaths::addResourceType("ko_effects", "data", "/effects/");
    KoResourcePaths::addResourceType("tags", "data", "/tags/");
    KoResourcePaths::addResourceType("templates", "data", "/templates");
    KoResourcePaths::addResourceType("pythonscripts", "data", "/pykrita");
    KoResourcePaths::addResourceType("symbols", "data", "/symbols");
    KoResourcePaths::addResourceType("preset_icons", "data", "/preset_icons");

    //    // Extra directories to look for create resources. (Does anyone actually use that anymore?)
    //    KoResourcePaths::addResourceDir("ko_gradients", "/usr/share/create/gradients/gimp");
    //    KoResourcePaths::addResourceDir("ko_gradients", QDir::homePath() + QString("/.create/gradients/gimp"));
    //    KoResourcePaths::addResourceDir("ko_patterns", "/usr/share/create/patterns/gimp");
    //    KoResourcePaths::addResourceDir("ko_patterns", QDir::homePath() + QString("/.create/patterns/gimp"));
    //    KoResourcePaths::addResourceDir("kis_brushes", "/usr/share/create/brushes/gimp");
    //    KoResourcePaths::addResourceDir("kis_brushes", QDir::homePath() + QString("/.create/brushes/gimp"));
    //    KoResourcePaths::addResourceDir("ko_palettes", "/usr/share/create/swatches");
    //    KoResourcePaths::addResourceDir("ko_palettes", QDir::homePath() + QString("/.create/swatches"));

    // Make directories for all resources we can save, and tags
    QDir d;
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/tags/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/asl/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/bundles/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/brushes/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/gradients/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/paintoppresets/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/palettes/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/patterns/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/taskset/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/workspaces/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/input/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/pykrita/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/symbols/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/color-schemes/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/preset_icons/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/preset_icons/tool_icons/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/preset_icons/emblem_icons/");
}

bool KisApplication::loadResources()
{
    KisResourceLoaderRegistry *reg = KisResourceLoaderRegistry::instance();

    reg->add(new KisResourceLoader<KisPaintOpPreset>("paintoppresets", "paintoppresets",  QStringList() << "application/x-krita-paintoppreset"));

    reg->add(new KisResourceLoader<KisGbrBrush>("gbr_brushes", "brushes", QStringList() << "image/x-gimp-brush"));
    reg->add(new KisResourceLoader<KisImagePipeBrush>("gih_brushes", "brushes", QStringList() << "image/x-gimp-brush-animated"));
    reg->add(new KisResourceLoader<KisSvgBrush>("svg_brushes", "brushes", QStringList() << "image/svg+xml"));
    reg->add(new KisResourceLoader<KisPngBrush>("png_brushes", "brushes", QStringList() << "image/png"));

    reg->add(new KisResourceLoader<KoSegmentGradient>("segmented_gradients", "gradients", QStringList() << "application/x-gimp-gradient"));
    reg->add(new KisResourceLoader<KoStopGradient>("stop_gradients", "gradients", QStringList() << "application/x-karbon-gradient" << "image/svg+xml"));

    reg->add(new KisResourceLoader<KoColorSet>("palettes", "palettes",
                                     QStringList() << KisMimeDatabase::mimeTypeForSuffix("kpl")
                                               << KisMimeDatabase::mimeTypeForSuffix("gpl")
                                               << KisMimeDatabase::mimeTypeForSuffix("pal")
                                               << KisMimeDatabase::mimeTypeForSuffix("act")
                                               << KisMimeDatabase::mimeTypeForSuffix("aco")
                                               << KisMimeDatabase::mimeTypeForSuffix("css")
                                               << KisMimeDatabase::mimeTypeForSuffix("colors")
                                               << KisMimeDatabase::mimeTypeForSuffix("xml")
                                               << KisMimeDatabase::mimeTypeForSuffix("sbz")));

    QList<QByteArray> src = QImageReader::supportedMimeTypes();
    QStringList allImageMimes;
    Q_FOREACH(const QByteArray ba, src) {
        allImageMimes << QString::fromUtf8(ba);
    }
    allImageMimes << KisMimeDatabase::mimeTypeForSuffix("pat");

    reg->add(new KisResourceLoader<KoPattern>("patterns", "patterns", allImageMimes));
    reg->add(new KisResourceLoader<KisWorkspaceResource>("workspaces", "workspaces", QStringList() << "application/x-krita-workspace"));
    reg->add(new KisResourceLoader<KoSvgSymbolCollectionResource>("symbols", "symbols", QStringList() << "image/svg+xml"));
    reg->add(new KisResourceLoader<KisSessionResource>("windowlayouts", "sessions", QStringList() << "application/x-krita-windowlayout"));
    reg->add(new KisResourceLoader<KisSessionResource>("sessions", "sessions", QStringList() << "application/x-krita-session"));

    if (!KisResourceCacheDb::initialize(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))) {
        QMessageBox::critical(0, i18nc("@title:window", "Krita: Fatal error"), i18n("Could not create the resources cache database. Krita will quit now."));
        return false;
    }

    KisResourceLocator::LocatorError r = KisResourceLocator::instance()->initialize(KoResourcePaths::getApplicationRoot() + "/share/krita");
    connect(KisResourceLocator::instance(), SIGNAL(progressMessage(const QString&)), this, SLOT(setSplashScreenLoadingText(const QString&)));
    if (r != KisResourceLocator::LocatorError::Ok ) {
        QMessageBox::critical(0, i18nc("@title:window", "Krita: Fatal error"), KisResourceLocator::instance()->errorMessages().join('\n') + i18n("\n\nKrita will quit now."));
        return false;
    }

    setSplashScreenLoadingText(i18n("Loading Resources..."));
    processEvents();
    KoResourceServerProvider::instance();

    setSplashScreenLoadingText(i18n("Loading Brush Presets..."));
    processEvents();
    KisResourceServerProvider::instance();

    setSplashScreenLoadingText(i18n("Loading Brushes..."));
    processEvents();
    KisBrushServer::instance()->brushServer();

    setSplashScreenLoadingText(i18n("Loading Bundles..."));
    processEvents();
    KisResourceBundleServerProvider::instance();

    return true;
}

void KisApplication::loadResourceTags()
{
    //    qDebug() << "loadResourceTags()";

    KoResourceServerProvider::instance()->patternServer()->loadTags();
    KoResourceServerProvider::instance()->gradientServer()->loadTags();
    KoResourceServerProvider::instance()->paletteServer()->loadTags();
    KoResourceServerProvider::instance()->svgSymbolCollectionServer()->loadTags();
    KisBrushServer::instance()->brushServer()->loadTags();
    KisResourceServerProvider::instance()->workspaceServer()->loadTags();
    KisResourceServerProvider::instance()->layerStyleCollectionServer()->loadTags();
    KisResourceBundleServerProvider::instance()->resourceBundleServer()->loadTags();
    KisResourceServerProvider::instance()->paintOpPresetServer()->loadTags();

    KisResourceServerProvider::instance()->paintOpPresetServer()->clearOldSystemTags();
}

void KisApplication::loadPlugins()
{
    //    qDebug() << "loadPlugins();";

    KoShapeRegistry* r = KoShapeRegistry::instance();
    r->add(new KisShapeSelectionFactory());

    KisActionRegistry::instance();
    KisFilterRegistry::instance();
    KisGeneratorRegistry::instance();
    KisPaintOpRegistry::instance();
    KoColorSpaceRegistry::instance();
}

void KisApplication::loadGuiPlugins()
{
    //    qDebug() << "loadGuiPlugins();";
    // Load the krita-specific tools
    setSplashScreenLoadingText(i18n("Loading Plugins for Krita/Tool..."));
    processEvents();
    //    qDebug() << "loading tools";
    KoPluginLoader::instance()->load(QString::fromLatin1("Krita/Tool"),
                                     QString::fromLatin1("[X-Krita-Version] == 28"));


    // Load dockers
    setSplashScreenLoadingText(i18n("Loading Plugins for Krita/Dock..."));
    processEvents();
    //    qDebug() << "loading dockers";
    KoPluginLoader::instance()->load(QString::fromLatin1("Krita/Dock"),
                                     QString::fromLatin1("[X-Krita-Version] == 28"));

    // XXX_EXIV: make the exiv io backends real plugins
    setSplashScreenLoadingText(i18n("Loading Plugins Exiv/IO..."));
    processEvents();
    //    qDebug() << "loading exiv2";
    KisExiv2::initialize();
}

bool KisApplication::start(const KisApplicationArguments &args)
{
    KisConfig cfg(false);

#if defined(Q_OS_WIN)
#ifdef ENV32BIT

    if (isWow64() && !cfg.readEntry("WarnedAbout32Bits", false)) {
        QMessageBox::information(0,
                                 i18nc("@title:window", "Krita: Warning"),
                                 i18n("You are running a 32 bits build on a 64 bits Windows.\n"
                                      "This is not recommended.\n"
                                      "Please download and install the x64 build instead."));
        cfg.writeEntry("WarnedAbout32Bits", true);

    }
#endif
#endif

    QString opengl = cfg.canvasState();
    if (opengl == "OPENGL_NOT_TRIED" ) {
        cfg.setCanvasState("TRY_OPENGL");
    }
    else if (opengl != "OPENGL_SUCCESS") {
        cfg.setCanvasState("OPENGL_FAILED");
    }

    setSplashScreenLoadingText(i18n("Initializing Globals"));
    processEvents();
    initializeGlobals(args);

    const bool doNewImage = args.doNewImage();
    const bool doTemplate = args.doTemplate();
    const bool exportAs = args.exportAs();
    const QString exportFileName = args.exportFileName();

    d->batchRun = (exportAs || !exportFileName.isEmpty());
    const bool needsMainWindow = !exportAs;
    // only show the mainWindow when no command-line mode option is passed
    bool showmainWindow = !exportAs; // would be !batchRun;

    const bool showSplashScreen = !d->batchRun && qEnvironmentVariableIsEmpty("NOSPLASH");
    if (showSplashScreen && d->splashScreen) {
        d->splashScreen->show();
        d->splashScreen->repaint();
        processEvents();
    }

    KoHashGeneratorProvider::instance()->setGenerator("MD5", new KisMD5Generator());

    KConfigGroup group(KSharedConfig::openConfig(), "theme");
    Digikam::ThemeManager themeManager;
    themeManager.setCurrentTheme(group.readEntry("Theme", "Krita dark"));

    ResetStarting resetStarting(d->splashScreen, args.filenames().count()); // remove the splash when done
    Q_UNUSED(resetStarting);

    // Make sure we can save resources and tags
    setSplashScreenLoadingText(i18n("Adding resource types"));
    processEvents();
    addResourceTypes();

    // Load the plugins
    loadPlugins();

    // Load all resources
    if (!loadResources()) {
        return false;
    }

    // Load all the tags
    loadResourceTags();

    // Load the gui plugins
    loadGuiPlugins();

    KisPart *kisPart = KisPart::instance();
    if (needsMainWindow) {
        // show a mainWindow asap, if we want that
        setSplashScreenLoadingText(i18n("Loading Main Window..."));
        processEvents();


        bool sessionNeeded = true;
        auto sessionMode = cfg.sessionOnStartup();

        if (!args.session().isEmpty()) {
            sessionNeeded = !kisPart->restoreSession(args.session());
        } else if (sessionMode == KisConfig::SOS_ShowSessionManager) {
            showmainWindow = false;
            sessionNeeded = false;
            kisPart->showSessionManager();
        } else if (sessionMode == KisConfig::SOS_PreviousSession) {
            KConfigGroup sessionCfg = KSharedConfig::openConfig()->group("session");
            const QString &sessionName = sessionCfg.readEntry("previousSession");

            sessionNeeded = !kisPart->restoreSession(sessionName);
        }

        if (sessionNeeded) {
            kisPart->startBlankSession();
        }

        if (!args.windowLayout().isEmpty()) {
            KoResourceServer<KisWindowLayoutResource> * rserver = KisResourceServerProvider::instance()->windowLayoutServer();
            KisWindowLayoutResource* windowLayout = rserver->resourceByName(args.windowLayout());
            if (windowLayout) {
                windowLayout->applyLayout();
            }
        }

        if (showmainWindow) {
            d->mainWindow = kisPart->currentMainwindow();

            if (!args.workspace().isEmpty()) {
                KoResourceServer<KisWorkspaceResource> * rserver = KisResourceServerProvider::instance()->workspaceServer();
                KisWorkspaceResource* workspace = rserver->resourceByName(args.workspace());
                if (workspace) {
                    d->mainWindow->restoreWorkspace(workspace);
                }
            }

            if (args.canvasOnly()) {
                d->mainWindow->viewManager()->switchCanvasOnly(true);
            }

            if (args.fullScreen()) {
                d->mainWindow->showFullScreen();
            }
        } else {
            d->mainWindow = kisPart->createMainWindow();
        }
    }
    short int numberOfOpenDocuments = 0; // number of documents open

    // Check for autosave files that can be restored, if we're not running a batchrun (test)
    if (!d->batchRun) {
        checkAutosaveFiles();
    }

    setSplashScreenLoadingText(QString()); // done loading, so clear out label
    processEvents();

    //configure the unit manager
    KisSpinBoxUnitManagerFactory::setDefaultUnitManagerBuilder(new KisDocumentAwareSpinBoxUnitManagerBuilder());
    connect(this, &KisApplication::aboutToQuit, &KisSpinBoxUnitManagerFactory::clearUnitManagerBuilder); //ensure the builder is destroyed when the application leave.
    //the new syntax slot syntax allow to connect to a non q_object static method.


    // Create a new image, if needed
    if (doNewImage) {
        KisDocument *doc = args.image();
        if (doc) {
            kisPart->addDocument(doc);
            d->mainWindow->addViewAndNotifyLoadingCompleted(doc);
        }
    }

    // Get the command line arguments which we have to parse
    int argsCount = args.filenames().count();
    if (argsCount > 0) {
        // Loop through arguments
        for (int argNumber = 0; argNumber < argsCount; argNumber++) {
            QString fileName = args.filenames().at(argNumber);
            // are we just trying to open a template?
            if (doTemplate) {
                // called in mix with batch options? ignore and silently skip
                if (d->batchRun) {
                    continue;
                }
                if (createNewDocFromTemplate(fileName, d->mainWindow)) {
                    ++numberOfOpenDocuments;
                }
                // now try to load
            }
            else {
                if (exportAs) {
                    QString outputMimetype = KisMimeDatabase::mimeTypeForFile(exportFileName, false);
                    if (outputMimetype == "application/octetstream") {
                        dbgKrita << i18n("Mimetype not found, try using the -mimetype option") << endl;
                        return 1;
                    }

                    KisDocument *doc = kisPart->createDocument();
                    doc->setFileBatchMode(d->batchRun);
                    doc->openUrl(QUrl::fromLocalFile(fileName));

                    qApp->processEvents(); // For vector layers to be updated

                    doc->setFileBatchMode(true);
                    if (!doc->exportDocumentSync(QUrl::fromLocalFile(exportFileName), outputMimetype.toLatin1())) {
                        dbgKrita << "Could not export " << fileName << "to" << exportFileName << ":" << doc->errorMessage();
                    }
                    QTimer::singleShot(0, this, SLOT(quit()));
                }
                else if (d->mainWindow) {
                    if (fileName.endsWith(".bundle")) {
                        d->mainWindow->installBundle(fileName);
                    }
                    else {
                        KisMainWindow::OpenFlags flags = d->batchRun ? KisMainWindow::BatchMode : KisMainWindow::None;

                        if (d->mainWindow->openDocument(QUrl::fromLocalFile(fileName), flags)) {
                            // Normal case, success
                            numberOfOpenDocuments++;
                        }
                    }
                }
            }
        }
    }

    // fixes BUG:369308  - Krita crashing on splash screen when loading.
    // trying to open a file before Krita has loaded can cause it to hang and crash
    if (d->splashScreen) {
        d->splashScreen->displayLinks(true);
        d->splashScreen->displayRecentFiles(true);
    }


    // not calling this before since the program will quit there.
    return true;
}

KisApplication::~KisApplication()
{
}

void KisApplication::setSplashScreen(QWidget *splashScreen)
{
    d->splashScreen = qobject_cast<KisSplashScreen*>(splashScreen);
}

void KisApplication::setSplashScreenLoadingText(const QString &textToLoad)
{
    if (d->splashScreen) {
        d->splashScreen->setLoadingText(textToLoad);
        d->splashScreen->repaint();
    }
}

void KisApplication::hideSplashScreen()
{
    if (d->splashScreen) {
        // hide the splashscreen to see the dialog
        d->splashScreen->hide();
    }
}


bool KisApplication::notify(QObject *receiver, QEvent *event)
{
    try {
        return QApplication::notify(receiver, event);
    } catch (std::exception &e) {
        qWarning("Error %s sending event %i to object %s",
                 e.what(), event->type(), qPrintable(receiver->objectName()));
    } catch (...) {
        qWarning("Error <unknown> sending event %i to object %s",
                 event->type(), qPrintable(receiver->objectName()));
    }
    return false;
}


void KisApplication::remoteArguments(QByteArray message, QObject *socket)
{
    Q_UNUSED(socket);

    // check if we have any mainwindow
    KisMainWindow *mw = qobject_cast<KisMainWindow*>(qApp->activeWindow());
    if (!mw) {
        mw = KisPart::instance()->mainWindows().first();
    }

    if (!mw) {
        return;
    }

    KisApplicationArguments args = KisApplicationArguments::deserialize(message);
    const bool doTemplate = args.doTemplate();
    const int argsCount = args.filenames().count();

    if (argsCount > 0) {
        // Loop through arguments
        for (int argNumber = 0; argNumber < argsCount; ++argNumber) {
            QString filename = args.filenames().at(argNumber);
            // are we just trying to open a template?
            if (doTemplate) {
                createNewDocFromTemplate(filename, mw);
            }
            else if (QFile(filename).exists()) {
                KisMainWindow::OpenFlags flags = d->batchRun ? KisMainWindow::BatchMode : KisMainWindow::None;
                mw->openDocument(QUrl::fromLocalFile(filename), flags);
            }
        }
    }
}

void KisApplication::fileOpenRequested(const QString &url)
{
    KisMainWindow *mainWindow = KisPart::instance()->mainWindows().first();
    if (mainWindow) {
        KisMainWindow::OpenFlags flags = d->batchRun ? KisMainWindow::BatchMode : KisMainWindow::None;
        mainWindow->openDocument(QUrl::fromLocalFile(url), flags);
    }
}


void KisApplication::checkAutosaveFiles()
{
    if (d->batchRun) return;

    // Check for autosave files from a previous run. There can be several, and
    // we want to offer a restore for every one. Including a nice thumbnail!

    QStringList filters;
    filters << QString(".krita-*-*-autosave.kra");

#ifdef Q_OS_WIN
    QDir dir = QDir::temp();
#else
    QDir dir = QDir::home();
#endif

    // all autosave files for our application
    QStringList autosaveFiles = dir.entryList(filters, QDir::Files | QDir::Hidden);

    // Allow the user to make their selection
    if (autosaveFiles.size() > 0) {
        if (d->splashScreen) {
            // hide the splashscreen to see the dialog
            d->splashScreen->hide();
        }
        d->autosaveDialog = new KisAutoSaveRecoveryDialog(autosaveFiles, activeWindow());
        QDialog::DialogCode result = (QDialog::DialogCode) d->autosaveDialog->exec();

        if (result == QDialog::Accepted) {
            QStringList filesToRecover = d->autosaveDialog->recoverableFiles();
            Q_FOREACH (const QString &autosaveFile, autosaveFiles) {
                if (!filesToRecover.contains(autosaveFile)) {
                    QFile::remove(dir.absolutePath() + "/" + autosaveFile);
                }
            }
            autosaveFiles = filesToRecover;
        } else {
            autosaveFiles.clear();
        }

        if (autosaveFiles.size() > 0) {
            QList<QUrl> autosaveUrls;
            Q_FOREACH (const QString &autoSaveFile, autosaveFiles) {
                const QUrl url = QUrl::fromLocalFile(dir.absolutePath() + QLatin1Char('/') + autoSaveFile);
                autosaveUrls << url;
            }
            if (d->mainWindow) {
                Q_FOREACH (const QUrl &url, autosaveUrls) {
                    KisMainWindow::OpenFlags flags = d->batchRun ? KisMainWindow::BatchMode : KisMainWindow::None;
                    d->mainWindow->openDocument(url, flags | KisMainWindow::RecoveryFile);
                }
            }
        }
        // cleanup
        delete d->autosaveDialog;
        d->autosaveDialog = nullptr;
    }
}

bool KisApplication::createNewDocFromTemplate(const QString &fileName, KisMainWindow *mainWindow)
{
    QString templatePath;

    const QUrl templateUrl = QUrl::fromLocalFile(fileName);
    if (QFile::exists(fileName)) {
        templatePath = templateUrl.toLocalFile();
        dbgUI << "using full path...";
    }
    else {
        QString desktopName(fileName);
        const QString templatesResourcePath =  QStringLiteral("templates/");

        QStringList paths = KoResourcePaths::findAllResources("data", templatesResourcePath + "*/" + desktopName);
        if (paths.isEmpty()) {
            paths = KoResourcePaths::findAllResources("data", templatesResourcePath + desktopName);
        }

        if (paths.isEmpty()) {
            QMessageBox::critical(0, i18nc("@title:window", "Krita"),
                                  i18n("No template found for: %1", desktopName));
        } else if (paths.count() > 1) {
            QMessageBox::critical(0, i18nc("@title:window", "Krita"),
                                  i18n("Too many templates found for: %1", desktopName));
        } else {
            templatePath = paths.at(0);
        }
    }

    if (!templatePath.isEmpty()) {
        QUrl templateBase;
        templateBase.setPath(templatePath);
        KDesktopFile templateInfo(templatePath);

        QString templateName = templateInfo.readUrl();
        QUrl templateURL;
        templateURL.setPath(templateBase.adjusted(QUrl::RemoveFilename|QUrl::StripTrailingSlash).path() + '/' + templateName);

        KisMainWindow::OpenFlags batchFlags = d->batchRun ? KisMainWindow::BatchMode : KisMainWindow::None;
        if (mainWindow->openDocument(templateURL, KisMainWindow::Import | batchFlags)) {
            dbgUI << "Template loaded...";
            return true;
        }
        else {
            QMessageBox::critical(0, i18nc("@title:window", "Krita"),
                                  i18n("Template %1 failed to load.", templateURL.toDisplayString()));
        }
    }

    return false;
}

void KisApplication::clearConfig()
{
    KIS_ASSERT_RECOVER_RETURN(qApp->thread() == QThread::currentThread());

    KSharedConfigPtr config =  KSharedConfig::openConfig();

    // find user settings file
    bool createDir = false;
    QString kritarcPath = KoResourcePaths::locateLocal("config", "kritarc", createDir);

    QFile configFile(kritarcPath);
    if (configFile.exists()) {
        // clear file
        if (configFile.open(QFile::WriteOnly)) {
            configFile.close();
        }
        else {
            QMessageBox::warning(0,
                                 i18nc("@title:window", "Krita"),
                                 i18n("Failed to clear %1\n\n"
                                      "Please make sure no other program is using the file and try again.",
                                      kritarcPath),
                                 QMessageBox::Ok, QMessageBox::Ok);
        }
    }

    // reload from disk; with the user file settings cleared,
    // this should load any default configuration files shipping with the program
    config->reparseConfiguration();
    config->sync();
}

void KisApplication::askClearConfig()
{
    Qt::KeyboardModifiers mods = QApplication::queryKeyboardModifiers();
    bool askClearConfig = (mods & Qt::ControlModifier) && (mods & Qt::ShiftModifier) && (mods & Qt::AltModifier);

    if (askClearConfig) {
        bool ok = QMessageBox::question(0,
                                        i18nc("@title:window", "Krita"),
                                        i18n("Do you want to clear the settings file?"),
                                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;
        if (ok) {
            clearConfig();
        }
    }
}
