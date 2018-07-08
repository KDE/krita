/*
 *  Copyright (c) 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "LogDockerDock.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QScrollBar>
#include <QStandardPaths>
#include <QDateTime>
#include <QTableWidget>

#include <klocalizedstring.h>

#include <KoDialog.h>
#include <KoCanvasBase.h>
#include <KoIcon.h>
#include <KoFileDialog.h>

#include "kis_canvas2.h"
#include "KisViewManager.h"
#include "kis_config.h"

QTextEdit *LogDockerDock::s_textEdit {0};
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

    s_textEdit = txtLogViewer;

    bnToggle->setIcon(koIcon("view-list-text"));
    connect(bnToggle, SIGNAL(clicked(bool)), SLOT(toggleLogging(bool)));
    bnToggle->setChecked(KisConfig().readEntry<bool>("logviewer_enabled", false));
    toggleLogging(KisConfig().readEntry<bool>("logviewer_enabled", false));

    bnClear->setIcon(koIcon("edit-clear"));
    connect(bnClear, SIGNAL(clicked(bool)), SLOT(clearLog()));

    bnSave->setIcon(koIcon("document-save"));
    connect(bnSave, SIGNAL(clicked(bool)), SLOT(saveLog()));

    bnSettings->setIcon(koIcon("settings"));
    connect(bnSettings, SIGNAL(clicked(bool)), SLOT(settings()));

    s_debug.setForeground(Qt::white);
    s_info.setForeground(Qt::yellow);
    s_warning.setForeground(QColor(255, 80, 0));
    s_critical.setForeground(Qt::red);
    s_fatal.setForeground(Qt::red);
    s_fatal.setFontWeight(QFont::Bold);
}

void LogDockerDock::setCanvas(KoCanvasBase *canvas)
{
    Q_UNUSED(canvas);
    setEnabled(true);

}

void LogDockerDock::toggleLogging(bool toggle)
{
    KisConfig().writeEntry<bool>("logviewer_enabled", toggle);
    if (toggle) {
        qInstallMessageHandler(messageHandler);
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

    QTableWidget *page = new QTableWidget(18, 6, &dlg);
    dlg.setMainWidget(page);

    page->setHorizontalHeaderItem(0, new QTableWidgetItem(i18n("Category")));
    page->setHorizontalHeaderItem(1, new QTableWidgetItem(i18n("Debug")));
    page->setHorizontalHeaderItem(2, new QTableWidgetItem(i18n("Info")));
    page->setHorizontalHeaderItem(3, new QTableWidgetItem(i18n("Warning")));
    page->setHorizontalHeaderItem(4, new QTableWidgetItem(i18n("Critical")));
    page->setHorizontalHeaderItem(4, new QTableWidgetItem(i18n("Fatal")));

    if (dlg.exec()) {
        // Apply the new settings
    }

}

void LogDockerDock::messageHandler(QtMsgType type, const QMessageLogContext &/*context*/, const QString &msg)
{
    QTextDocument *doc = s_textEdit->document();
    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    cursor.beginEditBlock();

    QByteArray localMsg = msg.toLocal8Bit();
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
    s_textEdit->verticalScrollBar()->setValue(s_textEdit->verticalScrollBar()->maximum());
}



