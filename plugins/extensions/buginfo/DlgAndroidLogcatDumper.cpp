/*
 * SPDX-FileCopyrightText: 2022 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "DlgAndroidLogcatDumper.h"

#include <QDebug>
#include <QProcess>
#include <QPushButton>
#include <QScreen>
#include <QScrollBar>
#include <QStandardPaths>
#include <unistd.h>

DlgAndroidLogcatDumper::DlgAndroidLogcatDumper(QWidget *parent)
    : DlgBugInfo(parent, User3)
    , m_logcatProcess(parent)
    , m_updateWidgetCompressor(100, KisSignalCompressor::FIRST_INACTIVE)
{
    QString program = "logcat";
    QStringList arguments = {"-v", "time", "--pid", QString::number(getpid())};
    m_logcatProcess.start(program, arguments);
    QString errorOutput = m_logcatProcess.readAllStandardError();
    if (!errorOutput.isEmpty()) {
        qWarning() << "Couldn't spawn logcat process" << errorOutput;
    }

    setButtonText(User3, i18n("Toggle Logging"));
    button(User3)->setCheckable(true);
    button(User3)->setChecked(true);

    enableLogging();
    connect(this, &KoDialog::user3Clicked, this, [this]() {
        QPushButton *loggingButton = button(User3);
        if (loggingButton->isChecked()) {
            enableLogging();
        } else {
            disableLogging();
        }
    });

    QRect screen_rect = QGuiApplication::primaryScreen()->availableGeometry();
    int frame_height = parentWidget()->frameGeometry().height() - parentWidget()->size().height();
    resize(m_page->size().width(), screen_rect.height() - frame_height);
}

DlgAndroidLogcatDumper::~DlgAndroidLogcatDumper()
{
    m_logcatProcess.kill();
    m_logcatProcess.waitForFinished();
}

void DlgAndroidLogcatDumper::writeTextToWidget()
{
    QScrollBar *scrollBar = m_page->txtBugInfo->verticalScrollBar();
    const bool scrollbarAtBottom = scrollBar->value() >= scrollBar->maximum();

    QByteArray output = m_logcatProcess.readAllStandardOutput();

    // this allows user's movement of the scrollbar
    QTextCursor tmp(m_page->txtBugInfo->document());
    tmp.movePosition(QTextCursor::End);
    tmp.insertText(output);

    // scroll to the bottom if scrollbar is at the bottom
    if (scrollbarAtBottom) {
        scrollBar->setValue(scrollBar->maximum());
    }
}

void DlgAndroidLogcatDumper::enableLogging()
{
    connect(&m_logcatProcess, SIGNAL(readyReadStandardOutput()), &m_updateWidgetCompressor, SLOT(start()),
            Qt::UniqueConnection);
    connect(&m_updateWidgetCompressor, SIGNAL(timeout()), this, SLOT(writeTextToWidget()), Qt::UniqueConnection);

    m_updateWidgetCompressor.start();
}

void DlgAndroidLogcatDumper::disableLogging()
{
    disconnect(&m_updateWidgetCompressor, SIGNAL(timeout()), this, SLOT(writeTextToWidget()));
    disconnect(&m_logcatProcess, SIGNAL(readyReadStandardOutput()), &m_updateWidgetCompressor, SLOT(start()));
}

QString DlgAndroidLogcatDumper::defaultNewFileName() { return "kritalogcatdump.txt"; }

QString DlgAndroidLogcatDumper::originalFileName()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/krita-logcatdump.log";
}

QString DlgAndroidLogcatDumper::captionText()
{
    return i18nc("Caption of the dialog with Krita's Android system log for bug reports",
                 "Krita Logcat Dump: please paste this information to the bug report");
}

QString DlgAndroidLogcatDumper::replacementWarningText() { return QString(); }
