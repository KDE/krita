/*
 *  Copyright (c) 2018 Jouni Pentikäinen <joupent@gmail.com>
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
#include <KisSessionResource.h>
#include <KisResourceServerProvider.h>
#include <QInputDialog>
#include <QMessageBox>
#include <KisPart.h>
#include "KisSessionManagerDialog.h"

KisSessionManagerDialog::KisSessionManagerDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    updateSessionList();

    connect(btnNew, SIGNAL(clicked()), this, SLOT(slotNewSession()));
    connect(btnRename, SIGNAL(clicked()), this, SLOT(slotRenameSession()));
    connect(btnSwitchTo, SIGNAL(clicked()), this, SLOT(slotSwitchSession()));
    connect(btnDelete, SIGNAL(clicked()), this, SLOT(slotDeleteSession()));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(slotClose()));

    connect(lstSessions, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slotSessionDoubleClicked(QListWidgetItem*)));
}

void KisSessionManagerDialog::updateSessionList() {
    KoResourceServer<KisSessionResource> *server = KisResourceServerProvider::instance()->sessionServer();

    lstSessions->clear();
    Q_FOREACH(KisSessionResourceSP session, server->resources()) {
        lstSessions->addItem(session->name());
    }
}

void KisSessionManagerDialog::slotNewSession()
{
    QString name = QInputDialog::getText(this,
        i18n("Create session"),
        i18n("Session name:"), QLineEdit::Normal
    );
    if (name.isNull() || name.isEmpty()) return;

    KisSessionResourceSP session(new KisSessionResource(QString()));

    KoResourceServer<KisSessionResource> *server = KisResourceServerProvider::instance()->sessionServer();
    QString saveLocation = server->saveLocation();
    QFileInfo fileInfo(saveLocation + name + session->defaultFileExtension());
    int i = 1;
    while (fileInfo.exists()) {
        fileInfo.setFile(saveLocation + name + QString("%1").arg(i) + session->defaultFileExtension());
        i++;
    }

    session->setFilename(fileInfo.filePath());
    session->setName(name);
    session->storeCurrentWindows();

    server->addResource(session);

    KisPart::instance()->setCurrentSession(session);

    updateSessionList();
}

void KisSessionManagerDialog::slotRenameSession()
{
    QString name = QInputDialog::getText(this,
         i18n("Rename session"),
         i18n("New name:"), QLineEdit::Normal
    );
    if (name.isNull() || name.isEmpty()) return;

    KisSessionResourceSP session = getSelectedSession();
    if (!session) return;

    session->setName(name);
    session->save();

    updateSessionList();
}

void KisSessionManagerDialog::slotSessionDoubleClicked(QListWidgetItem* /*item*/)
{
    slotSwitchSession();
    slotClose();
}

void KisSessionManagerDialog::slotSwitchSession()
{
    KisSessionResourceSP session = getSelectedSession();

    if (session) {
        bool closed = KisPart::instance()->closeSession(true);
        if (closed) {
            session->restore();
        }
    }
}

KisSessionResourceSP KisSessionManagerDialog::getSelectedSession() const
{
    QListWidgetItem *item = lstSessions->currentItem();
    if (item) {
        KoResourceServer<KisSessionResource> *server = KisResourceServerProvider::instance()->sessionServer();
        return server->resourceByName(item->text());
    }
    return nullptr;
}

void KisSessionManagerDialog::slotDeleteSession()
{
    KisSessionResourceSP session = getSelectedSession();
    if (!session) return;

    if (QMessageBox::warning(this,
        i18nc("@title:window", "Krita"),
        QString(i18n("Permanently delete session %1?", session->name())),
        QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {

        const QString filename = session->filename();

        KoResourceServer<KisSessionResource> *server = KisResourceServerProvider::instance()->sessionServer();
        server->removeResourceFromServer(session);

        QFile file(filename);
        file.remove();

        updateSessionList();
    }
}

void KisSessionManagerDialog::slotClose()
{
    hide();
}
