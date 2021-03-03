/*
 *  SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include "LogDockerDock.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QScrollBar>
#include <QStandardPaths>
#include <QDateTime>
#include <QCheckBox>

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include <KisPart.h>
#include <KoDialog.h>
#include <KoCanvasBase.h>
#include <KoIcon.h>
#include <KoFileDialog.h>

#include "kis_canvas2.h"
#include "KisViewManager.h"
#include "kis_config.h"

MessageSender *LogDockerDock::s_messageSender {new MessageSender()};
QTextCharFormat LogDockerDock::s_debug;
QTextCharFormat LogDockerDock::s_info;
QTextCharFormat LogDockerDock::s_warning;
QTextCharFormat LogDockerDock::s_critical;
QTextCharFormat LogDockerDock::s_fatal;

LogDockerDock::LogDockerDock( )
    : QDockWidget(i18n("Log Viewer"))
{
    QWidget *page = new QWidget(this);
    setupUi(page);
    setWidget(page);

    bnToggle->setIcon(koIcon("view-list-text"));
    connect(bnToggle, SIGNAL(clicked(bool)), SLOT(toggleLogging(bool)));
    bnToggle->setChecked(KisConfig(true).readEntry<bool>("logviewer_enabled", false));
    toggleLogging(KisConfig(true).readEntry<bool>("logviewer_enabled", false));

    bnClear->setIcon(koIcon("edit-clear"));
    connect(bnClear, SIGNAL(clicked(bool)), SLOT(clearLog()));

    bnSave->setIcon(koIcon("document-save"));
    connect(bnSave, SIGNAL(clicked(bool)), SLOT(saveLog()));

    bnSettings->setIcon(koIcon("configure-thicker"));
    connect(bnSettings, SIGNAL(clicked(bool)), SLOT(settings()));

    qRegisterMetaType<QtMsgType>("QtMsgType");
    connect(s_messageSender, SIGNAL(emitMessage(QtMsgType,QString)), this, SLOT(insertMessage(QtMsgType,QString)), Qt::AutoConnection);

    applyCategories();
    changeTheme();
}

void LogDockerDock::setCanvas(KoCanvasBase *)
{
    setEnabled(true);
}

void LogDockerDock::setViewManager(KisViewManager *kisview)
{
    connect(static_cast<KisMainWindow*>(kisview->mainWindow()), SIGNAL(themeChanged()), SLOT(changeTheme()));
}

void LogDockerDock::toggleLogging(bool toggle)
{
    KisConfig(false).writeEntry<bool>("logviewer_enabled", toggle);
    if (toggle) {
        qInstallMessageHandler(messageHandler);
        applyCategories();
    }
    else {
        qInstallMessageHandler(0);
    }

}

void LogDockerDock::clearLog()
{
    txtLogViewer->document()->clear();
}

void LogDockerDock::saveLog()
{
    KoFileDialog fileDialog(this, KoFileDialog::SaveFile, "logfile");
    fileDialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/" + QString("krita_%1.log").arg(QDateTime::currentDateTime().toString()));
    QString filename = fileDialog.filename();
    if (!filename.isEmpty()) {
        QFile f(filename);
        f.open(QFile::WriteOnly);
        f.write(txtLogViewer->document()->toPlainText().toUtf8());
        f.close();
    }
}

void LogDockerDock::settings()
{
    KoDialog dlg(this);
    dlg.setButtons(KoDialog::Ok | KoDialog::Cancel);
    dlg.setCaption(i18n("Log Settings"));
    QWidget *page = new QWidget(&dlg);
    dlg.setMainWidget(page);
    QVBoxLayout *layout = new QVBoxLayout(page);

    KConfigGroup cfg( KSharedConfig::openConfig(), "LogDocker");

    QCheckBox *chkKrita = new QCheckBox(i18n("General"), page);
    chkKrita->setChecked(cfg.readEntry("krita_41000", false));
    layout->addWidget(chkKrita);

    QCheckBox *chkResources = new QCheckBox(i18n("Resource Management"), page);
    chkResources->setChecked(cfg.readEntry("resources_30009", false));
    layout->addWidget(chkResources);

    QCheckBox *chkImage = new QCheckBox(i18n("Image Core"), page);
    chkImage->setChecked(cfg.readEntry("image_41001", false));
    layout->addWidget(chkImage);

    QCheckBox *chkRegistry = new QCheckBox(i18n("Registries"), page);
    chkRegistry->setChecked(cfg.readEntry("registry_41002", false));
    layout->addWidget(chkRegistry);

    QCheckBox *chkTools = new QCheckBox(i18n("Tools"), page);
    chkTools->setChecked(cfg.readEntry("tools_41003", false));
    layout->addWidget(chkTools);

    QCheckBox *chkTiles = new QCheckBox(i18n("Tile Engine"), page);
    chkTiles->setChecked(cfg.readEntry("tiles_41004", false));
    layout->addWidget(chkTiles);

    QCheckBox *chkFilters = new QCheckBox(i18n("Filters"), page);
    chkFilters->setChecked(cfg.readEntry("filters_41005", false));
    layout->addWidget(chkFilters);

    QCheckBox *chkPlugins = new QCheckBox(i18n("Plugin Management"), page);
    chkPlugins->setChecked(cfg.readEntry("plugins_41006", false));
    layout->addWidget(chkPlugins);

    QCheckBox *chkUi = new QCheckBox(i18n("User Interface"), page);
    chkUi->setChecked(cfg.readEntry("ui_41007", false));
    layout->addWidget(chkUi);

    QCheckBox *chkFile = new QCheckBox(i18n("File loading and saving"), page);
    chkFile->setChecked(cfg.readEntry("file_41008", false));
    layout->addWidget(chkFile);

    QCheckBox *chkMath = new QCheckBox(i18n("Mathematics and calculations"), page);
    chkMath->setChecked(cfg.readEntry("math_41009", false));
    layout->addWidget(chkMath);

    QCheckBox *chkRender = new QCheckBox(i18n("Image Rendering"), page);
    chkRender->setChecked(cfg.readEntry("render_41010", false));
    layout->addWidget(chkRender);

    QCheckBox *chkScript = new QCheckBox(i18n("Scripting"), page);
    chkScript->setChecked(cfg.readEntry("script_41011", false));
    layout->addWidget(chkScript);

    QCheckBox *chkInput = new QCheckBox(i18n("Input handling"), page);
    chkInput->setChecked(cfg.readEntry("input_41012", false));
    layout->addWidget(chkInput);

    QCheckBox *chkAction = new QCheckBox(i18n("Actions"), page);
    chkAction->setChecked(cfg.readEntry("action_41013", false));
    layout->addWidget(chkAction);

    QCheckBox *chkTablet = new QCheckBox(i18n("Tablet Handling"), page);
    chkTablet->setChecked(cfg.readEntry("tablet_41014", false));
    layout->addWidget(chkTablet);

    QCheckBox *chkOpenGL = new QCheckBox(i18n("GPU Canvas"), page);
    chkOpenGL->setChecked(cfg.readEntry("opengl_41015", false));
    layout->addWidget(chkOpenGL);

    QCheckBox *chkMetaData = new QCheckBox(i18n("Metadata"), page);
    chkMetaData->setChecked(cfg.readEntry("metadata_41016", false));
    layout->addWidget(chkMetaData);

    QCheckBox *chkPigment = new QCheckBox(i18n("Color Management"), page);
    chkPigment->setChecked(cfg.readEntry("pigment", false));
    layout->addWidget(chkPigment);


    if (dlg.exec()) {
        // Apply the new settings
        cfg.writeEntry("resources_30009", chkResources->isChecked());
        cfg.writeEntry("krita_41000", chkKrita->isChecked());
        cfg.writeEntry("image_41001", chkImage->isChecked());
        cfg.writeEntry("registry_41002", chkRegistry->isChecked());
        cfg.writeEntry("tools_41003", chkTools->isChecked());
        cfg.writeEntry("tiles_41004", chkTiles->isChecked());
        cfg.writeEntry("filters_41005", chkFilters->isChecked());
        cfg.writeEntry("plugins_41006", chkPlugins->isChecked());
        cfg.writeEntry("ui_41007", chkUi->isChecked());
        cfg.writeEntry("file_41008", chkFile->isChecked());
        cfg.writeEntry("math_41009", chkMath->isChecked());
        cfg.writeEntry("render_41010", chkRender->isChecked());
        cfg.writeEntry("script_41011", chkScript->isChecked());
        cfg.writeEntry("input_41012", chkInput->isChecked());
        cfg.writeEntry("action_41013", chkAction->isChecked());
        cfg.writeEntry("tablet_41014", chkTablet->isChecked());
        cfg.writeEntry("opengl_41015", chkOpenGL->isChecked());
        cfg.writeEntry("metadata_41016", chkMetaData->isChecked());
        cfg.writeEntry("pigment", chkPigment->isChecked());

        applyCategories();
    }

}

QString cfgToString(QString tpl, bool cfg)
{
    return tpl.arg(cfg ? "true" : "false");
}

void LogDockerDock::applyCategories()
{
    QStringList filters;
    KConfigGroup cfg( KSharedConfig::openConfig(), "LogDocker");

    filters << cfgToString("krita.general=%1", cfg.readEntry("krita_41000", false));
    filters << cfgToString("krita.lib.resources=%1", cfg.readEntry("resources_30009", false));
    filters << cfgToString("krita.core=%1", cfg.readEntry("image_41001", false));
    filters << cfgToString("krita.registry=%1", cfg.readEntry("registry_41002", false));

    filters << cfgToString("krita.tools=%1", cfg.readEntry("tools_41003", false));
    filters << cfgToString("krita.lib.flake=%1", cfg.readEntry("tools_41003", false));

    filters << cfgToString("krita.tiles=%1", cfg.readEntry("tiles_41004", false));
    filters << cfgToString("krita.filters=%1", cfg.readEntry("filters_41005", false));

    filters << cfgToString("krita.plugins=%1", cfg.readEntry("plugins_41006", false));
    filters << cfgToString("krita.lib.plugin=%1", cfg.readEntry("plugins_41006", false));

    filters << cfgToString("krita.ui=%1", cfg.readEntry("ui_41007", false));
    filters << cfgToString("krita.widgets=%1", cfg.readEntry("ui_41007", false));
    filters << cfgToString("krita.widgetutils=%1", cfg.readEntry("ui_41007", false));

    filters << cfgToString("krita.file=%1", cfg.readEntry("file_41008", false));
    filters << cfgToString("krita.lib.store=%1", cfg.readEntry("file_41008", false));
    filters << cfgToString("krita.lib.odf=%1", cfg.readEntry("file_41008", false));

    filters << cfgToString("krita.math=%1", cfg.readEntry("math_41009", false));
    filters << cfgToString("krita.grender=%1", cfg.readEntry("render_41010", false));
    filters << cfgToString("krita.scripting=%1", cfg.readEntry("script_41011", false));
    filters << cfgToString("krita.input=%1", cfg.readEntry("input_41012", false));
    filters << cfgToString("krita.action=%1", cfg.readEntry("action_41013", false));
    filters << cfgToString("krita.tablet=%1", cfg.readEntry("tablet_41014", false));
    filters << cfgToString("krita.opengl=%1", cfg.readEntry("opengl_41015", false));
    filters << cfgToString("krita.metadata=%1", cfg.readEntry("metadata_41016", false));

    filters << cfgToString("krita.lib.pigment=%1", cfg.readEntry("pigment", false));

    QLoggingCategory::setFilterRules(filters.join("\n"));
}

void LogDockerDock::messageHandler(QtMsgType type, const QMessageLogContext &/*context*/, const QString &msg)
{
    s_messageSender->sendMessage(type, msg);
}

void LogDockerDock::insertMessage(QtMsgType type, const QString &msg)
{
    QTextDocument *doc = txtLogViewer->document();
    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    cursor.beginEditBlock();

    switch (type) {
    case QtDebugMsg:
        cursor.insertText(msg + "\n", s_debug);
        break;
    case QtInfoMsg:
        cursor.insertText(msg + "\n", s_info);
        break;
    case QtWarningMsg:
        cursor.insertText(msg + "\n", s_warning);
        break;
    case QtCriticalMsg:
        cursor.insertText(msg + "\n", s_critical);
        break;
    case QtFatalMsg:
        cursor.insertText(msg + "\n", s_fatal);
        break;
    }

    cursor.endEditBlock();
    txtLogViewer->verticalScrollBar()->setValue(txtLogViewer->verticalScrollBar()->maximum());
}

void LogDockerDock::changeTheme()
{
    clearLog();
    QColor background = qApp->palette().window().color();
    if (background.value() > 100) {
        s_debug.setForeground(Qt::black);
        s_info.setForeground(Qt::darkGreen);
        s_warning.setForeground(Qt::darkYellow);
        s_critical.setForeground(Qt::darkRed);
        s_fatal.setForeground(Qt::darkRed);
    }
    else {
        s_debug.setForeground(Qt::white);
        s_info.setForeground(Qt::green);
        s_warning.setForeground(Qt::yellow);
        s_critical.setForeground(Qt::red);
        s_fatal.setForeground(Qt::red);
    }
    s_fatal.setFontWeight(QFont::Bold);
}

void MessageSender::sendMessage(QtMsgType type, const QString &msg)
{
    emit emitMessage(type, msg);
}
