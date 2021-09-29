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
#include <KisResourceOverwriteDialog.h>
#include <KisResourceModel.h>

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

    name = QInputDialog::getText(this,
                                 i18n("Create session"),
                                 i18n("Session name:"), QLineEdit::Normal,
                                 name);

    KisSessionResourceSP session(new KisSessionResource(QString(name)));

    QString filename = name.split(" ").join("_") + session->defaultFileExtension();
    session->setFilename(filename);
    session->setName(name);
    session->storeCurrentWindows();

    KisResourceModel resourceModel(ResourceType::Sessions);
    KisResourceOverwriteDialog::addResourceWithUserInput(this, &resourceModel, session);

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

    KisResourceModel resourceModel(ResourceType::Sessions);
    KisResourceOverwriteDialog::renameResourceWithUserInput(this, &resourceModel, session, name);
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
        KoResourceSP res = m_model->resourceForIndex(idx); // lstSessions uses the same index as m_model
        return res.dynamicCast<KisSessionResource>();
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
