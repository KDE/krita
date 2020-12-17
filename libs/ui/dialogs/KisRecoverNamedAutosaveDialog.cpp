/*
 *  SPDX-FileCopyrightText: 2020 Agata Cacko <cacko.azh@gmail.com>>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisRecoverNamedAutosaveDialog.h"
#include "ui_KisRecoverNamedAutosaveDialog.h"

#include <QTimer>
#include <QElapsedTimer>
#include <QThread>

#include "kis_debug.h"
#include "kis_image.h"
#include "kis_composite_progress_proxy.h"

#include <KisFileIconCreator.h>


KisRecoverNamedAutosaveDialog::KisRecoverNamedAutosaveDialog(QWidget *parent, QString mainFile, QString autosaveFile)
    : QDialog(parent),
      ui(new Ui::KisRecoverNamedAutosaveDialog)
{

    ui->setupUi(this);

    connect(ui->btOk, SIGNAL(clicked()), this, SLOT(slotOkRequested()));
    connect(ui->btCancel, SIGNAL(clicked()), this, SLOT(slotCancelRequested()));

    ui->lblExplanation->setText(i18nc("Recover an autosave for an already existing file: explanation in the recovery dialog",
                                      "An autosave for this file exists. How do you want to proceed?\n"
                                      "Warning: if you discard the autosave now, it will be removed."));

    KisFileIconCreator creator;
    QIcon mainFileIcon, autosaveFileIcon;

    QSize size = ui->rbOpenAutosave->iconSize();
    size = size*4;
    bool success = creator.createFileIcon(mainFile, mainFileIcon, devicePixelRatioF(), size);
    if (success) {
        ui->rbDiscardAutosave->setIcon(mainFileIcon);
        ui->rbDiscardAutosave->setIconSize(size);
    }
    success = creator.createFileIcon(autosaveFile, autosaveFileIcon, devicePixelRatioF(), size);
    if (success) {
        ui->rbOpenAutosave->setIcon(autosaveFileIcon);
        ui->rbOpenAutosave->setIconSize(size);
    }

    ui->rbOpenAutosave->setChecked(true); // it should be selected by default

}

KisRecoverNamedAutosaveDialog::~KisRecoverNamedAutosaveDialog()
{
    delete ui;
}

void KisRecoverNamedAutosaveDialog::slotOkRequested()
{
    close();
    setResult(ui->rbOpenAutosave->isChecked() ? OpenAutosave : OpenMainFile);
}

void KisRecoverNamedAutosaveDialog::slotCancelRequested()
{
    close();
    setResult(Cancel);
}

