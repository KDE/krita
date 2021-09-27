/*
 *  SPDX-FileCopyrightText: 2018 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <KisSessionResource.h>
#include <KisResourceServerProvider.h>
#include <QInputDialog>
#include <QMessageBox>
#include <KisPart.h>
#include "KisSessionManagerDialog.h"

int KisSessionManagerDialog::refreshEventType = -1;

KisSessionManagerDialog::KisSessionManagerDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    
    // Register the custom event type that is used to defer UI updates
    if (refreshEventType == -1) {
        refreshEventType = QEvent::registerEventType();
    }

    connect(btnNew, SIGNAL(clicked()), this, SLOT(slotNewSession()));
    connect(btnRename, SIGNAL(clicked()), this, SLOT(slotRenameSession()));
    connect(btnSwitchTo, SIGNAL(clicked()), this, SLOT(slotSwitchSession()));
    connect(btnDelete, SIGNAL(clicked()), this, SLOT(slotDeleteSession()));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(slotClose()));

    m_model = new KisResourceModel(ResourceType::Sessions, this);
    lstSessions->setModel(m_model);
    lstSessions->setModelColumn(KisAbstractResourceModel::Name);

    connect(lstSessions, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotSessionDoubleClicked(QModelIndex)));
    
    connect(lstSessions->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(slotModelSelectionChanged(QItemSelection, QItemSelection)));

    updateButtons();
}

bool KisSessionManagerDialog::event(QEvent *event)
{
    if (event->type() == (QEvent::Type) refreshEventType) {
        // Do the actual work of updating the button state when receiving a custom event
        bool hasSelectedSession = getSelectedSession() != nullptr;
        btnDelete->setEnabled(hasSelectedSession);
        btnSwitchTo->setEnabled(hasSelectedSession);
        btnRename->setEnabled(hasSelectedSession);
        return true;
    } else {
        return QDialog::event(event);
    }
}

void KisSessionManagerDialog::updateButtons()
{
    // Defer updating the buttons by posting a custom event with low priority to avoid locking against
    // a non-recursive session lock that may be already held by the thread
    QApplication::postEvent(this, new QEvent((QEvent::Type) refreshEventType), Qt::LowEventPriority);
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

    bool r = server->addResource(session);
    if (!r) {
        QMessageBox::warning(this, i18nc("@title:window", "Couldn't add the session"), i18nc("Warning message", "Adding the session to the database failed."));
    }

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

    bool r = m_model->renameResource(session, name);
    if (!r) {
        QMessageBox::warning(this, i18nc("@title:window", "Couldn't rename the session"),
                             i18nc("Warning about not being able to rename a session", "Renaming the session failed."));
    }
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
        QString md5 = m_model->data(idx, Qt::UserRole + KisAbstractResourceModel::MD5).toString();
        return server->resource(md5, "", "");
    }
    return nullptr;
}

void KisSessionManagerDialog::slotDeleteSession()
{
    QModelIndex idx = lstSessions->currentIndex();
    if (idx.isValid()) {
        m_model->setResourceInactive(lstSessions->currentIndex());
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
        m_lastSessionId = m_model->data(idx, Qt::UserRole + KisAbstractResourceModel::Id).toInt();
    }
}

void KisSessionManagerDialog::slotModelReset()
{
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex idx = m_model->index(i, 0);
        int id = m_model->data(idx, Qt::UserRole + KisAbstractResourceModel::Id).toInt();
        if (id == m_lastSessionId) {
            lstSessions->setCurrentIndex(idx);
        }
    }
    
    updateButtons();
}

void KisSessionManagerDialog::slotModelSelectionChanged(QItemSelection selected, QItemSelection deselected)
{
    (void) selected;
    (void) deselected;
    
    updateButtons();
}
