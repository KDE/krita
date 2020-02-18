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

    connect(btnNew, SIGNAL(clicked()), this, SLOT(slotNewSession()));
    connect(btnRename, SIGNAL(clicked()), this, SLOT(slotRenameSession()));
    connect(btnSwitchTo, SIGNAL(clicked()), this, SLOT(slotSwitchSession()));
    connect(btnDelete, SIGNAL(clicked()), this, SLOT(slotDeleteSession()));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(slotClose()));


    m_model = KisResourceModelProvider::resourceModel(ResourceType::Sessions);
    lstSessions->setModel(m_model);
    lstSessions->setModelColumn(KisResourceModel::Name);


    connect(m_model, SIGNAL(beforeResourcesLayoutReset(QModelIndex)), this, SLOT(slotModelAboutToBeReset(QModelIndex)));
    connect(m_model, SIGNAL(afterResourcesLayoutReset()), this, SLOT(slotModelReset()));

    connect(lstSessions, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotSessionDoubleClicked(QModelIndex)));


}

void KisSessionManagerDialog::slotNewSession()
{
    QString name;

    KisSessionResourceSP session(new KisSessionResource(QString()));

    KoResourceServer<KisSessionResource> *server = KisResourceServerProvider::instance()->sessionServer();
    QString saveLocation = server->saveLocation();
    QFileInfo fileInfo(saveLocation + name.split(" ").join("_") + session->defaultFileExtension());

    bool fileOverwriteAccepted = false;

    while(!fileOverwriteAccepted) {
        name = QInputDialog::getText(this,
                                     i18n("Create session"),
                                     i18n("Session name:"), QLineEdit::Normal,
                                     name);
        if (name.isNull() || name.isEmpty()) {
            return;
        } else {
            fileInfo = QFileInfo(saveLocation + name.split(" ").join("_") + session->defaultFileExtension());
            if (fileInfo.exists()) {
                int res = QMessageBox::warning(this, i18nc("@title:window", "Name Already Exists")
                                                        , i18n("The name '%1' already exists, do you wish to overwrite it?", name)
                                                        , QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                if (res == QMessageBox::Yes) fileOverwriteAccepted = true;
            } else {
                fileOverwriteAccepted = true;
            }
        }
    }

    session->setFilename(fileInfo.fileName());
    session->setName(name);
    session->storeCurrentWindows();

    server->addResource(session);

    KisPart::instance()->setCurrentSession(session);
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
    m_model->updateResource(session);
    session->save();
}

void KisSessionManagerDialog::slotSessionDoubleClicked(QModelIndex /*item*/)
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
            KisPart::instance()->restoreSession(session);
        }
    }
}

KisSessionResourceSP KisSessionManagerDialog::getSelectedSession() const
{
    QModelIndex idx = lstSessions->currentIndex();
    if (idx.isValid()) {
        KoResourceServer<KisSessionResource> *server = KisResourceServerProvider::instance()->sessionServer();
        QString name = m_model->data(idx, Qt::UserRole + KisResourceModel::Name).toString();
        return server->resourceByName(name);
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

        KisPart::instance()->setCurrentSession(0);
        const QString filename = session->filename();

        KoResourceServer<KisSessionResource> *server = KisResourceServerProvider::instance()->sessionServer();
        server->removeResourceFromServer(session);

        QFile file(filename);
        file.remove();
    }
}

void KisSessionManagerDialog::slotClose()
{
    hide();
}

void KisSessionManagerDialog::slotModelAboutToBeReset(QModelIndex)
{
    QModelIndex idx = lstSessions->currentIndex();
    if (idx.isValid()) {
        m_lastSessionId = m_model->data(idx, Qt::UserRole + KisResourceModel::Id).toInt();
    }
}

void KisSessionManagerDialog::slotModelReset()
{
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex idx = m_model->index(i, 0);
        int id = m_model->data(idx, Qt::UserRole + KisResourceModel::Id).toInt();
        if (id == m_lastSessionId) {
            lstSessions->setCurrentIndex(idx);
        }
    }
}
