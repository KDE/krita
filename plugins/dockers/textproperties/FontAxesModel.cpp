/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "FontAxesModel.h"
#include <QDebug>

const QString WEIGHT_TAG = "wght";
const QString WIDTH_TAG = "wdth";
const QString SLANT_TAG = "slnt";
const QString ITALIC_TAG = "ital";
const QString OPTICAL_TAG = "opsz";

struct FontAxesModel::Private {
    QList<KoSvgText::FontFamilyAxis> axes;
    QList<QLocale> locales;
    bool opticalSizeDisabled = false;
    QVariantHash axisValues;
};

FontAxesModel::FontAxesModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d(new Private)
{

}

FontAxesModel::~FontAxesModel()
{
}

void FontAxesModel::setAxesData(QList<KoSvgText::FontFamilyAxis> axes)
{
    beginResetModel();
    d->axes.clear();
    for (int i=0; i < axes.size(); i++) {
        QString tag = axes.at(i).tag;
        if (!(tag == WEIGHT_TAG
                || tag == WIDTH_TAG
                || tag == ITALIC_TAG
                || tag == SLANT_TAG)) {
            d->axes.append(axes.at(i));
        }
    }
    endResetModel();
}

void FontAxesModel::setLocales(QList<QLocale> locales)
{
    d->locales = locales;
}

void FontAxesModel::setOpticalSizeDisabled(bool disable)
{
    d->opticalSizeDisabled = disable;
}

QModelIndex FontAxesModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (column != 0) return QModelIndex();
    if (row >= 0 && row < d->axes.size()) return createIndex(row, column, &row);
    return QModelIndex();
}

QModelIndex FontAxesModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

int FontAxesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->axes.size();
}

int FontAxesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant FontAxesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    KoSvgText::FontFamilyAxis axis = d->axes.at(index.row());
    if (role == Qt::DisplayRole) {
        if (axis.localizedLabels.isEmpty()) return axis.tag;
        QString label = axis.localizedLabels.value("en", axis.localizedLabels.values().first());
        Q_FOREACH(const QLocale &locale, d->locales) {
            QString bcp = locale.bcp47Name();
            if (axis.localizedLabels.keys().contains(bcp)) {
                label = axis.localizedLabels.value(bcp, label);
                break;
            }
        }
        return label;
    } else if (role == Qt::EditRole) {
        return d->axisValues.value(axis.tag, axis.defaultValue).toDouble();
    } else if (role == Min) {
        return axis.min;
    } else if (role == Max) {
        return axis.max;
    } else if (role == Hidden) {
        return axis.axisHidden;
    }
    return QVariant();
}

bool FontAxesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {

        KoSvgText::FontFamilyAxis axis = d->axes.at(index.row());
        if (!d->axisValues.keys().contains(axis.tag) || d->axisValues.value(axis.tag) != value) {
            d->axisValues.insert(axis.tag, value);
            emit axisValuesChanged();
            emit dataChanged(index, index, { Qt::EditRole});
            return true;
        }
    }
    return false;
}

Qt::ItemFlags FontAxesModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index) | Qt::ItemNeverHasChildren | Qt::ItemIsEditable;

    /*KoSvgText::FontFamilyAxis axis = d->axes.value(index.row());
    if (!(axis.tag == OPTICAL_TAG && d->opticalSizeDisabled)) {
        flags |= Qt::ItemIsEnabled;
    }*/

    return flags;
}

QHash<int, QByteArray> FontAxesModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[Min] = "axismin";
    roles[Max] = "axismax";
    roles[Hidden] = "axishidden";
    return roles;
}

QVariantHash FontAxesModel::axisValues() const
{
    return d->axisValues;
}

void FontAxesModel::setAxisValues(const QVariantHash &newAxisValues)
{
    if (d->axisValues == newAxisValues)
        return;
    d->axisValues = newAxisValues;
    emit axisValuesChanged();
    emit dataChanged(index(0, 0, QModelIndex()), index(d->axes.size()-1, 0, QModelIndex()), {Qt::EditRole});
}
