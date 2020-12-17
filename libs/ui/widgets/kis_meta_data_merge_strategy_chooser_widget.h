/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2011 Boudewijn Rempot <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
