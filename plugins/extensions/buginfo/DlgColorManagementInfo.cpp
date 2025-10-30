/*
 * SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "DlgColorManagementInfo.h"
#include <QStandardPaths>

#include <KisMainWindow.h>

#include <KoPluginLoader.h>
#include <kpluginfactory.h>
#include <surfacecolormanagement/KisOutputColorInfoInterface.h>
#include <surfacecolormanagement/KisSurfaceColorManagementInfo.h>


DlgColorManagementInfo::DlgColorManagementInfo(QWidget *parent)
    : DlgBugInfo(parent)
{
    initialize();

    KPluginFactory *factory = KoPluginLoader::instance()->loadSinglePlugin(
        std::make_pair("X-Krita-PlatformId", QGuiApplication::platformName()),
        "Krita/PlatformPlugin");

    if (factory) {
        m_outputColorInfoInterface.reset(
            factory->create<KisOutputColorInfoInterface>(nullptr));

        if (m_outputColorInfoInterface) {
            connect(m_outputColorInfoInterface.data(), &KisOutputColorInfoInterface::sigReadyChanged,
                    this, &DlgColorManagementInfo::initializeText);

            connect(m_outputColorInfoInterface.data(), &KisOutputColorInfoInterface::sigOutputDescriptionChanged,
                    this, [this] () {
                        if (m_outputColorInfoInterface->isReady()) {
                            initializeText();
                        }
                    });
        }

        m_surfaceColorManagementInfo.reset(
            factory->create<KisSurfaceColorManagementInfo>(nullptr));

        if (m_surfaceColorManagementInfo) {
            auto future = m_surfaceColorManagementInfo->debugReport();
            future.then([this] (const QString &report) {
                m_surfaceManagementReport = report;
                initializeText();
            });
        }
    }
}

QString DlgColorManagementInfo::originalFileName()
{
    return "";
}

QString DlgColorManagementInfo::defaultNewFileName()
{
    return "KritaColorManagementInformation.txt";
}

QString DlgColorManagementInfo::captionText()
{
    return i18nc("Caption of the dialog with color management information for bug reports", "Krita Color Management Information: please paste this information to the bug report");
}

QString DlgColorManagementInfo::replacementWarningText()
{
    qFatal("not used");
    return "";
}

#include <QWindow>

#include <KisViewManager.h>
#include <KisDocument.h>
#include <kis_canvas2.h>
#include <KisPlatformPluginInterfaceFactory.h>

QString DlgColorManagementInfo::infoText(QSettings& kritarc)
{
    Q_UNUSED(kritarc)

    QString report;
    QDebug s(&report);

    s << "Main Window" << Qt::endl;
    s << "===" << Qt::endl;
    s << Qt::endl;

    KisMainWindow *mainWindow = qobject_cast<KisMainWindow*>(parent());

    s << "Native window handle:" << mainWindow->windowHandle() << Qt::endl;
    s << Qt::endl;

    s.noquote().nospace() << KisPlatformPluginInterfaceFactory::instance()->colorManagementReport(mainWindow);
    s.space().quote();
    s << Qt::endl;

    if (mainWindow->viewManager()->document()) {
        auto *doc = mainWindow->viewManager()->document();
        auto *canvas = mainWindow->viewManager()->canvasBase();

        s << "Document:" << doc->objectName() << "path:" << doc->localFilePath() << Qt::endl;
        s << "===" << Qt::endl;
        s << Qt::endl;

        s << "Native window handle:" << canvas->canvasWidget()->windowHandle() << Qt::endl;


        if (KisPlatformPluginInterfaceFactory::instance()->surfaceColorManagedByOS() &&
            canvas->canvasWidget()->windowHandle() == mainWindow->windowHandle()) {

                s << "WARNING: the canvas shares the surface with the main window on a platform with managed surface color space!";
        }
        s << Qt::endl;

        s.noquote().nospace() << canvas->colorManagementReport();
        s.space().quote();

        Q_FOREACH (QScreen *screen, qApp->screens()) {
            s << Qt::endl;
            s << "Screen:" << screen->name() << screen->manufacturer() << screen->model();
            if (screen == mainWindow->screen()) {
                s << "[CURRENT]";
            }
            s << Qt::endl;
            s << "===" << Qt::endl;
            s << Qt::endl;

            std::optional<KisSurfaceColorimetry::SurfaceDescription> desc;

            if (m_outputColorInfoInterface && m_outputColorInfoInterface->isReady()) {
                desc = m_outputColorInfoInterface->outputDescription(screen);
            }

            if (desc) {
                s.noquote().nospace() << desc->makeTextReport();
                s.space().quote();
            } else {
                s << "<no information available>";
            }
            s << Qt::endl;
        }
    }

    s << "Color management plugin report" << Qt::endl;
    s << "===" << Qt::endl;
    s << Qt::endl;

    s.noquote().nospace() << (m_surfaceManagementReport.isEmpty() ? "<no information available>" : m_surfaceManagementReport);
    s.space().quote();
    s << Qt::endl;

    return report;
}

DlgColorManagementInfo::~DlgColorManagementInfo()
{
}
