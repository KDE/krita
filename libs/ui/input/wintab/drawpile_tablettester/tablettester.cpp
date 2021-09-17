/*
   Drawpile - a collaborative drawing program.

   SPDX-FileCopyrightText: 2017 Calle Laakkonen

   SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "tablettester.h"
#include "tablettest.h"
#include "ui_tablettest.h"

TabletTestDialog::TabletTestDialog(QWidget *parent)
    : KoDialog(parent, Qt::Dialog)
{
    setCaption(i18n("Tablet Tester"));
    QWidget *page = new QWidget(this);
    m_ui = new Ui_TabletTest;
    m_ui->setupUi(page);
    setMainWidget(page);
    setButtons(KoDialog::Close);
    qApp->installEventFilter(this);

    m_ui->logView->appendPlainText(
                "## Legend:\n"
                "# X,Y - event coordinate\n"
                "# B - buttons pressed\n"
                "# S - speed\n"
                "\n");
}

TabletTestDialog::~TabletTestDialog()
{
    qApp->removeEventFilter(this);
    delete m_ui;
}

bool TabletTestDialog::eventFilter(QObject *watched, QEvent *e) {
    Q_UNUSED(watched);
    if(e->type() == QEvent::TabletEnterProximity || e->type() == QEvent::TabletLeaveProximity) {
        QTabletEvent *te = static_cast<QTabletEvent*>(e);
        bool isEraser = te->pointerType() == QTabletEvent::Eraser;
        bool isNear = e->type() == QEvent::TabletEnterProximity;
        QString msg;
        if(isEraser) {
            if (isNear) {
                msg = QStringLiteral("Eraser brought near");
            } else {
                msg = QStringLiteral("Eraser taken away");
            }
        } else {
            if (isNear) {
                msg = QStringLiteral("Pen tip brought near");
            } else {
                msg = QStringLiteral("Pen tip taken away");
            }
        }

        m_ui->logView->appendPlainText(msg);
    }
    return QDialog::eventFilter(watched, e);
}
