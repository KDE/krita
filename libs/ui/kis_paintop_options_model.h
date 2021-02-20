/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassyqweb.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KIS_PAINTOP_OPTION_LIST_MODEL_H_
#define _KIS_PAINTOP_OPTION_LIST_MODEL_H_

#include <kis_categorized_list_model.h>
#include <kis_paintop_option.h>
#include <kritaui_export.h>

#include <QString>

struct KRITAUI_EXPORT KisOptionInfo
{
    KisOptionInfo() = default;

    KisOptionInfo(KisPaintOpOption* o, int i, const QString &label)
        : label(label)
        , option(o)
        , index(i)
    {}

    KisOptionInfo(const KisOptionInfo &) = default;

    QString label;
    KisPaintOpOption *option {nullptr};
    int index;
};

KRITAUI_EXPORT bool operator==(const KisOptionInfo& a, const KisOptionInfo& b);

struct KRITAUI_EXPORT OptionInfoToQStringConverter {
    QString operator() (const KisOptionInfo &info) {
        return info.label;
    }
};

typedef KisCategorizedListModel<KisOptionInfo, OptionInfoToQStringConverter> BaseOptionCategorizedListModel;

/**
 * This model can be use to show a list of visible composite op in a list view.
 */
class KRITAUI_EXPORT KisPaintOpOptionListModel : public BaseOptionCategorizedListModel
{
public:
    KisPaintOpOptionListModel(QObject *parent);
    void addPaintOpOption(KisPaintOpOption* option, int widgetIndex, const QString &label, KisPaintOpOption::PaintopCategory categoryType);
    void addPaintOpOption(KisPaintOpOption *option, int widgetIndex, const QString &label, const QString &category);
    QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& idx, const QVariant& value, int role=Qt::EditRole) override;
    void signalDataChanged(const QModelIndex& index);
};

#endif // _KIS_PAINTOP_OPTION_LIST_MODEL_H_
