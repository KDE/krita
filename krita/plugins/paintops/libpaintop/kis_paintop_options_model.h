/* This file is part of the KDE project
 * Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2011 Silvio Heinrich <plassyqweb.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_PAINTOP_OPTION_LIST_MODEL_H_
#define _KIS_PAINTOP_OPTION_LIST_MODEL_H_

#include <kis_categorized_list_model.h>
#include <kis_paintop_option.h>

struct KisOptionInfo
{
    KisOptionInfo() { }
    KisOptionInfo(KisPaintOpOption* o, int i): option(o), index(i) { }
    KisOptionInfo(const KisOptionInfo& info): option(info.option), index(info.index) { }
    KisPaintOpOption* option;
    int               index;
};

struct OptionInfoToQStringConverter {
    QString operator() (const KisOptionInfo &info) {
        return info.option->label();
    }
};

typedef KisCategorizedListModel<KisOptionInfo, OptionInfoToQStringConverter> BaseOptionCategorizedListModel;

/**
 * This model can be use to show a list of visible composite op in a list view.
 */
class KisPaintOpOptionListModel : public BaseOptionCategorizedListModel
{
public:
    KisPaintOpOptionListModel(QObject *parent);
    void addPaintOpOption(KisPaintOpOption* option, int widgetIndex);
    virtual bool setData(const QModelIndex& idx, const QVariant& value, int role=Qt::EditRole);
};

#endif // _KIS_PAINTOP_OPTION_LIST_MODEL_H_
