/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
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

#include "kis_dlg_import_image_sequence.h"

#include "KisDocument.h"
#include "KisMainWindow.h"
#include "kis_image.h"
#include "kis_image_animation_interface.h"

class KisDlgImportImageSequence::ListItem : QListWidgetItem {

public:
    ListItem(const QString &text, QListWidget *view, QCollator *collator)
        : QListWidgetItem(text, view),
       collator(collator)
    {}

    bool operator <(const QListWidgetItem &other) const
    {
        int cmp = collator->compare(this->text(), other.text());
        return cmp < 0;
    }

private:
    QCollator *collator;
};

KisDlgImportImageSequence::KisDlgImportImageSequence(KisMainWindow *mainWindow, KisDocument *document) :
    KoDialog(mainWindow),
    mainWindow(mainWindow),
    document(document)
{
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    QWidget * page = new QWidget(this);
    ui.setupUi(page);
    setMainWidget(page);

    enableButtonOk(false);

    ui.cmbOrder->addItem(i18n("Ascending"), Ascending);
    ui.cmbOrder->addItem(i18n("Descending"), Descending);
    ui.cmbOrder->setCurrentIndex(0);

    ui.cmbSortMode->addItem(i18n("Alphabetical"), Natural);
    ui.cmbSortMode->addItem(i18n("Numerical"), Numerical);
    ui.cmbSortMode->setCurrentIndex(1);

    ui.lstFiles->setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(ui.btnAddImages, &QAbstractButton::clicked, this, &KisDlgImportImageSequence::slotAddFiles);
    connect(ui.btnRemove, &QAbstractButton::clicked, this, &KisDlgImportImageSequence::slotRemoveFiles);
    connect(ui.spinStep, SIGNAL(valueChanged(int)), this, SLOT(slotSkipChanged(int)));
    connect(ui.cmbOrder, SIGNAL(currentIndexChanged(int)), this, SLOT(slotOrderOptionsChanged(int)));
    connect(ui.cmbSortMode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotOrderOptionsChanged(int)));
}

QStringList KisDlgImportImageSequence::files()
{
    QStringList list;

    for (int i=0; i < ui.lstFiles->count(); i++) {
        list.append(ui.lstFiles->item(i)->text());
    }

    return list;
}

int KisDlgImportImageSequence::firstFrame()
{
    return ui.spinFirstFrame->value();
}

int KisDlgImportImageSequence::step()
{
    return ui.spinStep->value();
}

void KisDlgImportImageSequence::slotAddFiles()
{
    QStringList urls = mainWindow->showOpenFileDialog();

    if (!urls.isEmpty()) {
        Q_FOREACH(QString url, urls) {
            new ListItem(url, ui.lstFiles, &collator);
        }

        sortFileList();
    }

    enableButtonOk(ui.lstFiles->count() > 0);
}

void KisDlgImportImageSequence::slotRemoveFiles()
{
    QList<QListWidgetItem*> selected = ui.lstFiles->selectedItems();

    Q_FOREACH(QListWidgetItem *item, selected) {
        delete item;
    }

    enableButtonOk(ui.lstFiles->count() > 0);
}

void KisDlgImportImageSequence::slotSkipChanged(int)
{
    int documentFps = document->image()->animationInterface()->framerate();
    float sourceFps = 1.0f * documentFps / ui.spinStep->value();

    ui.lblFramerate->setText(i18n("Source fps: %1", sourceFps));
}

void KisDlgImportImageSequence::slotOrderOptionsChanged(int)
{
    sortFileList();
}

void KisDlgImportImageSequence::sortFileList()
{
    int order = ui.cmbOrder->currentData().toInt();
    bool numeric = ui.cmbSortMode->currentData().toInt() == Numerical;

    collator.setNumericMode(numeric);
    ui.lstFiles->sortItems((order == Ascending) ? Qt::AscendingOrder : Qt::DescendingOrder);
}
