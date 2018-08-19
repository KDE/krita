/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_meta_data_model.h"

#include <klocalizedstring.h>

#include <metadata/kis_meta_data_store.h>
#include <metadata/kis_meta_data_entry.h>
#include <metadata/kis_meta_data_value.h>

KisMetaDataModel::KisMetaDataModel(KisMetaData::Store* store) : m_store(store)
{

}

int KisMetaDataModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_store->keys().count();
}

int KisMetaDataModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant KisMetaDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    Q_ASSERT(index.row() < m_store->keys().count());
    switch (role) {
    case Qt::DisplayRole: {
        switch (index.column()) {
        case 0:
            return m_store->keys()[index.row()];
        case 1: {
            KisMetaData::Value::ValueType vt = m_store->entries()[index.row()].value().type();
            switch (vt) {
            case KisMetaData::Value::Invalid:
                return i18n("Invalid");
            case KisMetaData::Value::Variant: {
                int vt = m_store->entries()[index.row()].value().asVariant().type();
                switch (vt) {
                case QVariant::Date:
                case QVariant::DateTime:
                    return i18n("Date");
                case QVariant::Double:
                case QVariant::Int:
                    return i18n("Number");
                case QVariant::String:
                    return i18n("String");
                default:
                    return i18n("Variant (%1)", vt);
                }
            }
            case KisMetaData::Value::OrderedArray:
                return i18n("Ordered array");
            case KisMetaData::Value::UnorderedArray:
                return i18n("Unordered array");
            case KisMetaData::Value::AlternativeArray:
                return i18n("Alternative array");
            case KisMetaData::Value::LangArray:
                return i18n("Language array");
            case KisMetaData::Value::Structure:
                return i18n("Structure");
            case KisMetaData::Value::Rational:
                return i18n("Rational");
            }
            break;
        }
        case 2:
            return m_store->entries()[index.row()].value().toString();
        }
        break;
    }
    default:
        return QVariant();
    }
    return QVariant();
}

QVariant KisMetaDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        Q_ASSERT(section < 3);
        switch (section) {
        case 0:
            return i18n("Key");
        case 1:
            return i18n("Type");
        case 2:
            return i18nc("Metadata item value", "Value");
        }
    }
    return QVariant();
}
