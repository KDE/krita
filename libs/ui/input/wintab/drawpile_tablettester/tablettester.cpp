/*
   Drawpile - a collaborative drawing program.

   Copyright (C) 2017 Calle Laakkonen

   Drawpile is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Drawpile is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Drawpile.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "tablettester.h"
#include "tablettest.h"
#include "ui_tablettest.h"

TabletTestDialog::TabletTestDialog( QWidget *parent) :
    KoDialog(parent, Qt::Window)
{
    setCaption(i18n("Tablet Tester"));
    QWidget *page = new QWidget(this);
    m_ui = new Ui_TabletTest;
    m_ui->setupUi(page);
    setMainWidget(page);
    setButtons(KoDialog::Close);
    qApp->installEventFilter(this);
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
