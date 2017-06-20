/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2011 Boudewijn Rempot <boud@valdyas.org>
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

#ifndef _KIS_META_DATA_MERGE_STRATEGY_CHOOSER_WIDGET_H_
#define _KIS_META_DATA_MERGE_STRATEGY_CHOOSER_WIDGET_H_

#include <QWidget>

namespace KisMetaData
{
class MergeStrategy;
}

/**
 * This widget allows to select a meta data merge strategy.
 */
class KisMetaDataMergeStrategyChooserWidget : public QWidget
{
    Q_OBJECT
public:
    /**
     * Creates a widget to select a merge strategy.
     */
    KisMetaDataMergeStrategyChooserWidget(QWidget* parent);
    ~KisMetaDataMergeStrategyChooserWidget() override;

    const KisMetaData::MergeStrategy* currentStrategy();

    /**
     * Show a dialog which embed that widget, and have an Ok and Cancel button.
     * @return 0 if no merge strategy was selected, or the selected merge strategy
     */
    static const KisMetaData::MergeStrategy* showDialog(QWidget* parent);

private Q_SLOTS:

    void setCurrentDescription(int index);

private:

    const KisMetaData::MergeStrategy* mergeStrategy(int index);

    struct Private;
    Private* const d;
};

#endif
