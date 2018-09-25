/*
 *  Copyright (c) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
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
    QString gmicqt = "gmic_krita_qt";
#ifdef Q_OS_WIN
    gmicqt += ".exe";
#endif

    QString gmic_qt_path = KisConfig(true).readEntry<QString>("gmic_qt_plugin_path", "");
    if (!gmic_qt_path.isEmpty() && QFileInfo(gmic_qt_path).exists()) {
        return gmic_qt_path;
    }

    QFileInfo fi(qApp->applicationDirPath() + "/" + gmicqt);

    // Check for gmic-qt next to krita
    if (fi.exists() && fi.isFile()) {
//        dbgPlugins << 1 << fi.canonicalFilePath();
        return fi.canonicalFilePath();
    }

    // Check whether we've got a gmic subfolder
    QDir d(qApp->applicationDirPath());
    QStringList gmicdirs = d.entryList(QStringList() << "gmic*", QDir::Dirs);
    dbgPlugins << gmicdirs;
    if (gmicdirs.isEmpty()) {
//        dbgPlugins << 2;
        return "";
    }
    fi = QFileInfo(qApp->applicationDirPath() + "/" + gmicdirs.first() + "/" + gmicqt);
    if (fi.exists() && fi.isFile()) {
//        dbgPlugins << "3" << fi.canonicalFilePath();
        return fi.canonicalFilePath();
    }

//    dbgPlugins << 4 << gmicqt;
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
