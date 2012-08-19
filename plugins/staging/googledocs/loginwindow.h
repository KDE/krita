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

#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <onlinedocument.h>

#include <QDialog>

#include "ui_authenticationdialog.h"

class GoogleDocumentService;

namespace KWallet {
    class Wallet;
}

class LoginWindow : public QDialog
{
    Q_OBJECT

public:
    LoginWindow(OnlineDocument::DocumentType docType, QWidget *parent = 0);
    ~LoginWindow();
    GoogleDocumentService * googleService() {  return gdoc; }
    void showProgressIndicator(bool visible);

private slots:
    void loginService();
    void serviceSelected(int index);
    void authenticated(bool success, QString errorString);
    void updateProgress(QString msg);
    void closeWallet();

private:
    OnlineDocument::DocumentType m_type;
    Ui_Dialog *m_authDialog;
    GoogleDocumentService *gdoc;
    KWallet::Wallet *m_wallet;

    void saveUserDetails();
    KWallet::Wallet *wallet();
};

#endif // LOGINWINDOW_H
