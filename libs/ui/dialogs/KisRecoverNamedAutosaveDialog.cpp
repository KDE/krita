/*
 *  Copyright (c) 2020 Agata Cacko <cacko.azh@gmail.com>>
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
    bool success = creator.createFileIcon(mainFile, mainFileIcon, devicePixelRatioF());
    if (success) {
        ui->rbDiscardAutosave->setIcon(mainFileIcon);
        ui->rbDiscardAutosave->setIconSize(size);
    }
    success = creator.createFileIcon(autosaveFile, autosaveFileIcon, devicePixelRatioF());
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

