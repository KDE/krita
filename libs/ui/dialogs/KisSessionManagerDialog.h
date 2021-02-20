/*
 *  SPDX-FileCopyrightText: 2018 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSESSIONMANAGERDIALOG_H
#define KISSESSIONMANAGERDIALOG_H

#include <QDialog>


#include "ui_wdgsessionmanager.h"

#include <KisSessionResource.h>

class KisResourceModel;

class KisSessionManagerDialog : public QDialog, Ui::DlgSessionManager
{
    Q_OBJECT

public:
    explicit KisSessionManagerDialog(QWidget *parent = nullptr);
    
protected:
    bool event(QEvent *event) override;

private Q_SLOTS:
    void slotNewSession();
    void slotRenameSession();
    void slotSwitchSession();
    void slotDeleteSession();
    void slotSessionDoubleClicked(QModelIndex item);

    void slotClose();

    void slotModelAboutToBeReset(QModelIndex);
    void slotModelReset();

    void slotModelSelectionChanged(QItemSelection selected, QItemSelection deselected);

private:
    void updateButtons();

    KisSessionResourceSP getSelectedSession() const;

    int m_lastSessionId;

    KisResourceModel* m_model;
    
    static int refreshEventType;
};

#endif
