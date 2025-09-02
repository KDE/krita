/*
 * SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "DlgColorManagementInfo.h"
#include <QStandardPaths>

#include <KisMainWindow.h>


DlgColorManagementInfo::DlgColorManagementInfo(QWidget *parent)
    : DlgBugInfo(parent)
{
    initialize();
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

#include <KisViewManager.h>
#include <KisDocument.h>
#include <kis_canvas2.h>

QString DlgColorManagementInfo::infoText(QSettings& kritarc)
{
    Q_UNUSED(kritarc)

    QString report;
    QTextStream s(&report);

    s << "Main Window" << Qt::endl;
    s << "===" << Qt::endl;
    s << Qt::endl;

    KisMainWindow *mainWindow = qobject_cast<KisMainWindow*>(parent());
    s << mainWindow->colorManagementReport();

    if (mainWindow->viewManager()->document()) {
        auto *doc = mainWindow->viewManager()->document();
        auto *canvas = mainWindow->viewManager()->canvasBase();

        s << "Document: " << doc->objectName() << " path: " << doc->localFilePath() << Qt::endl;
        s << "===" << Qt::endl;
        s << Qt::endl;

        s << canvas->colorManagementReport();
    }

    return report;
}

DlgColorManagementInfo::~DlgColorManagementInfo()
{
}
