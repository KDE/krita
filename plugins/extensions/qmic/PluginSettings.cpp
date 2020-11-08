/*
 *  SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-only
 */
#include "PluginSettings.h"

#include <QVBoxLayout>
#include <QStandardPaths>
#include <QDir>

#include <kconfiggroup.h>
#include <klocalizedstring.h>

#include <KoIcon.h>

#include <kis_file_name_requester.h>
#include "kis_config.h"

PluginSettings::PluginSettings(QWidget *parent)
    : KisPreferenceSet(parent)
{
    setupUi(this);
    fileRequester->setFileName(gmicQtPath());
    fileRequester->setConfigurationName("gmic_qt");
    fileRequester->setStartDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
}

PluginSettings::~PluginSettings()
{
    KisConfig(false).writeEntry<QString>("gmic_qt_plugin_path", fileRequester->fileName());
}

QString PluginSettings::id()
{
    return QString("qmicsettings");
}

QString PluginSettings::name()
{
    return header();
}

QString PluginSettings::header()
{
    return QString(i18n("G'Mic-Qt Integration"));
}


QIcon PluginSettings::icon()
{
    return koIcon("gmic");
}

QString PluginSettings::gmicQtPath()
{
    QString gmicqt = "libgmic_krita_qt";
#if defined(Q_OS_WIN)
    gmicqt += ".dll";
#elif defined(Q_OS_MACOS)
    gmicqt += ".dylib";
#else
    gmicqt += ".so";
#endif

    QString gmic_qt_path = KisConfig(true).readEntry<QString>("gmic_qt_plugin_path", "");
    dbgPlugins << 1 << gmic_qt_path;
    if (!gmic_qt_path.isEmpty() && QFileInfo(gmic_qt_path).exists()) {
        return gmic_qt_path;
    }

    QFileInfo fi(qApp->applicationDirPath() + "/" + gmicqt);

    // Check for gmic-qt next to krita
    dbgPlugins << 2 << fi.filePath();
    if (fi.exists() && fi.isFile()) {
        return fi.canonicalFilePath();
    }

    // Check whether we've got a gmic subfolder
    QDir d(qApp->applicationDirPath());
    QStringList gmicdirs = d.entryList(QStringList() << "gmic*", QDir::Dirs);
    dbgPlugins << gmicdirs;

    for (const auto &gmicDir : gmicdirs) {
        fi = QFileInfo(qApp->applicationDirPath() + "/" + gmicDir + "/" + gmicqt);
        dbgPlugins << "3" << fi.filePath();
        if (fi.exists() && fi.isFile()) {
            return fi.canonicalFilePath();
        }
    }

    // Amyspark: check for gmic-qt plugin in the library path
    for (const auto &path : qApp->libraryPaths()) {
        QFileInfo fi(path + "/" + gmicqt);
        dbgPlugins << 4 << fi.filePath();
        if (fi.exists() && fi.isFile()) {
            return fi.canonicalFilePath();
        }
    }

    dbgPlugins << 5 << gmicqt;
    return gmicqt;
}


void PluginSettings::savePreferences() const
{
    KisConfig(false).writeEntry<QString>("gmic_qt_plugin_path", fileRequester->fileName());
    Q_EMIT(settingsChanged());
}

void PluginSettings::loadPreferences()
{
    fileRequester->setFileName(gmicQtPath());
}

void PluginSettings::loadDefaultPreferences()
{
    fileRequester->setFileName(gmicQtPath());
}
