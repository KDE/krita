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
#include "widgets/tablettest.h"
using widgets::TabletTester; // work around missing namespace in UIC generated code
#include "ui_tablettest.h"

#include "../main.h"

namespace dialogs {

TabletTestDialog::TabletTestDialog( QWidget *parent) :
	QDialog(parent)
{
	m_ui = new Ui_TabletTest;
	m_ui->setupUi(this);

	connect(static_cast<DrawpileApp*>(qApp), &DrawpileApp::eraserNear, this, [this](bool near) {
		QString msg;
		if(near)
			msg = QStringLiteral("Eraser brought near");
		else
			msg = QStringLiteral("Eraser taken away");

		m_ui->logView->appendPlainText(msg);
	});
}

TabletTestDialog::~TabletTestDialog()
{
	delete m_ui;
}

}

