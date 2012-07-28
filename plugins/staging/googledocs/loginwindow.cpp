/*
 *  Copyright (c) 2010 Mani Chandrasekar <maninc@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "loginwindow.h"
#include "googledocumentservice.h"
#include "documentlistwindow.h"

#include <QMessageBox>


LoginWindow::LoginWindow(OnlineDocument::DocumentType type, QWidget *parent)
	: QDialog(parent),
          m_type(type),
          m_authDialog(new Ui_Dialog)
{
    m_authDialog->setupUi(this);

    QStringList onlineServices;
    onlineServices << "Google Documents";
    // Add services here
    m_authDialog->comboBox->addItems(onlineServices);

    connect(m_authDialog->loginButton, SIGNAL(clicked()), this, SLOT(loginService()));
    connect(m_authDialog->comboBox, SIGNAL(activated(int)), this, SLOT(serviceSelected(int)));

    m_authDialog->userEdit->setFocus();
    showProgressIndicator(false);
    setWindowTitle("Online Document Services");
    show();
}

LoginWindow::~LoginWindow()
{
    delete m_authDialog;
}

void LoginWindow::loginService()
{
    if (0 == m_authDialog->comboBox->currentIndex()) {
        gdoc = new GoogleDocumentService(m_type);
        showProgressIndicator(true);
        m_authDialog->headerLabel->setText("Signing in...");
        gdoc->clientLogin(m_authDialog->userEdit->text(), m_authDialog->passwordEdit->text());
        connect(gdoc, SIGNAL(userAuthenticated(bool, QString)), this, SLOT(authenticated(bool, QString)));
        connect(gdoc, SIGNAL(progressUpdate(QString)), this, SLOT(updateProgress(QString)));
        connect(gdoc, SIGNAL(showingDocumentList()), this, SLOT(accept()));
    }
}

void LoginWindow::serviceSelected(int index)
{
//    if (index == 0) {
//        m_authDialog->documentBox->setVisible(true);
//        m_authDialog->presentationBox->setVisible(true);
//        m_authDialog->spreadsheetBox->setVisible(true);
//    } else if (index == 1) {
//        m_authDialog->documentBox->setVisible(false);
//        m_authDialog->presentationBox->setVisible(true);
//        m_authDialog->spreadsheetBox->setVisible(false);
//    }
}

void LoginWindow::authenticated(bool success, QString errorString)
{
    if (success) {
//        showProgressIndicator(false);
//        m_authDialog->label->setText("Successfully authenticated!!! Retreiving document list...");
//        accept();
    } else {
        QString msg = "Error occurred while signing in ";
        if (!errorString.isEmpty()) {
            msg = msg + "- " + errorString;
        }
        m_authDialog->headerLabel->setText(msg);
        showProgressIndicator(false);
    }
}

void LoginWindow::showProgressIndicator(bool visible)
{
    m_authDialog->progressBar->setVisible(visible);
}

void LoginWindow::updateProgress(QString msg)
{
    m_authDialog->headerLabel->setText(msg);
}
