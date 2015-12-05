/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "widgets/kis_meta_data_merge_strategy_chooser_widget.h"

#include <KoDialog.h>
#include <kis_debug.h>
#include <kis_meta_data_merge_strategy_registry.h>

#include "ui_wdgmetadatamergestrategychooser.h"

struct KisMetaDataMergeStrategyChooserWidget::Private {
    Ui::WdgMetaDataMergeStrategyChooser uiWdg;
};

KisMetaDataMergeStrategyChooserWidget::KisMetaDataMergeStrategyChooserWidget(QWidget* parent)
        : d(new Private)
{
    Q_UNUSED(parent);
    setObjectName("KisMetadataMergeStrategyChooserWidget");
    d->uiWdg.setupUi(this);
    QList<QString> keys = KisMetaData::MergeStrategyRegistry::instance()->keys();
    Q_FOREACH (const QString & key, keys) {
        const KisMetaData::MergeStrategy* ms = KisMetaData::MergeStrategyRegistry::instance()->get(key);
        d->uiWdg.mergeStrategy->addItem(ms->name(), ms->id());
    }
    int initial = d->uiWdg.mergeStrategy->findData("Smart");
    if (initial != -1) {
        d->uiWdg.mergeStrategy->setCurrentIndex(initial);
    }
    setCurrentDescription(d->uiWdg.mergeStrategy->currentIndex());
    connect(d->uiWdg.mergeStrategy, SIGNAL(currentIndexChanged(int)), SLOT(setCurrentDescription(int)));
}

KisMetaDataMergeStrategyChooserWidget::~KisMetaDataMergeStrategyChooserWidget()
{
    delete d;
}

const KisMetaData::MergeStrategy* KisMetaDataMergeStrategyChooserWidget::currentStrategy()
{
    return mergeStrategy(d->uiWdg.mergeStrategy->currentIndex());
}

void KisMetaDataMergeStrategyChooserWidget::setCurrentDescription(int index)
{
    d->uiWdg.description->setText(mergeStrategy(index)->description());
}

const KisMetaData::MergeStrategy* KisMetaDataMergeStrategyChooserWidget::mergeStrategy(int index)
{
    return KisMetaData::MergeStrategyRegistry::instance()->get(
               d->uiWdg.mergeStrategy->itemData(index).toString());
}

const KisMetaData::MergeStrategy* KisMetaDataMergeStrategyChooserWidget::showDialog(QWidget* parent)
{
    KoDialog dlg(parent);
    dlg.setCaption(i18n("Choose meta data merge strategy"));
    dlg.setButtons(KoDialog::Ok | KoDialog::Cancel);
    dlg.setDefaultButton(KoDialog::Ok);

    KisMetaDataMergeStrategyChooserWidget* wdg = new KisMetaDataMergeStrategyChooserWidget(&dlg);
    wdg->setMinimumSize(wdg->sizeHint());
    dlg.setMainWidget(wdg);
    if (dlg.exec() == QDialog::Accepted) {
        return wdg->currentStrategy();
    }

    return 0;
}

