/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "FontAxesModel.h"
#include <klocalizedstring.h>
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
    QVariantMap axisValues;

    bool blockAxesValuesUpdate = false;
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
    QList<KoSvgText::FontFamilyAxis> newAxes;
    for (int i=0; i < axes.size(); i++) {
        QString tag = axes.at(i).tag;
        if (!(tag == WEIGHT_TAG
                || tag == WIDTH_TAG
                || tag == ITALIC_TAG
                || tag == SLANT_TAG)
                && !axes.at(i).axisHidden) {
            newAxes.append(axes.at(i));
        }
    }
    if (newAxes != d->axes) {
        // TODO: Rework this whole thing so we don't update axes unless absolutely necessary.
        // This wll probably require a filterproxy model where invisible axes are filtered out.
        beginResetModel();
        d->axes = newAxes;
        endResetModel();
    }
}

void FontAxesModel::setLocales(QList<QLocale> locales)
{
    d->locales = locales;
}

void FontAxesModel::setOpticalSizeDisabled(bool disable)
{
    d->opticalSizeDisabled = disable;
}

void FontAxesModel::setBlockAxesValuesSignal(bool block)
{
    d->blockAxesValuesUpdate = block;
}

bool FontAxesModel::axesValueSignalBlocked() const
{
    return d->blockAxesValuesUpdate;
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
        if (axis.tag == OPTICAL_TAG){
            // Refers to https://learn.microsoft.com/en-us/typography/opentype/spec/dvaraxistag_opsz
            return i18nc("@info:label", "Optical Size");
        }
        if (axis.localizedLabels.isEmpty()) {
            return axis.tag;
        }
        QString label = axis.localizedLabels.value(QLocale(QLocale::English), axis.localizedLabels.values().first());
        Q_FOREACH(const QLocale &locale, d->locales) {
            if (axis.localizedLabels.keys().contains(locale)) {
                label = axis.localizedLabels.value(locale, label);
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
        if (!d->axisValues.keys().contains(axis.tag) || !qFuzzyCompare(d->axisValues.value(axis.tag).toDouble(), value.toDouble())) {
            d->axisValues.insert(axis.tag, value);
            if (!d->blockAxesValuesUpdate) {
                emit axisValuesChanged();
            }
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

QVariantMap FontAxesModel::axisValues() const
{
    return d->axisValues;
}

void FontAxesModel::setAxisValues(const QVariantMap &newAxisValues)
{
    if (d->axisValues == newAxisValues)
        return;
    d->axisValues = newAxisValues;

    if (!d->blockAxesValuesUpdate) {
        emit axisValuesChanged();
    }
    if (!d->axes.isEmpty()) {
        QModelIndex idx1 = index(0, 0, QModelIndex());
        QModelIndex idx2 = index(d->axes.size()-1, 0, QModelIndex());
        if (idx1.isValid() && idx2.isValid()) {
            emit dataChanged(idx1, idx2, {Qt::EditRole});
        }
    }

}
