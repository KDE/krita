/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <QList>

#include <KoFontGlyphModel.h>
#include <KoCSSFontInfo.h>
#include <KoOpenTypeFeatureInfoFactory.h>
#include <KoFontLibraryResourceUtils.h>
#include <KoSvgTextProperties.h>
#include <KoFontRegistry.h>
#include <lager/KoSvgTextPropertiesModel.h>
#include <KoSvgTextPropertyData.h>


#include "OpenTypeFeatureModel.h"
struct OpenTypeFeatureModel::Private {
    Private(QObject *parent)
        : glyphModel(new KoFontGlyphModel(parent))
        , allFeatures(new AllOpenTypeFeaturesModel(parent))
    {
    }

    QList<KoOpenTypeFeatureInfo> availableFeatures() const {
        if (glyphModel) {
            return glyphModel->featureInfo().values();
        }
        return QList<KoOpenTypeFeatureInfo>();
    }

    KoOpenTypeFeatureInfo featureByTag(QByteArray tag) const {
        KoOpenTypeFeatureInfo info = factory.infoByTag(tag);
        if (glyphModel) {
            info = glyphModel->featureInfo().value(tag, info);
        }
        return info;
    }

    KoFontGlyphModel *glyphModel {nullptr};
    AllOpenTypeFeaturesModel *allFeatures {nullptr};
    KoOpenTypeFeatureInfoFactory factory;
    KoCSSFontInfo fontInfo;
    QVariantMap currentFeatures;
};

OpenTypeFeatureModel::OpenTypeFeatureModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d(new Private(this))
{
}

OpenTypeFeatureModel::~OpenTypeFeatureModel()
{
}

QModelIndex OpenTypeFeatureModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (column != 0) return QModelIndex();
    if (row >= 0 && row < d->currentFeatures.size()) return createIndex(row, column, &row);
    return QModelIndex();
}

QModelIndex OpenTypeFeatureModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

int OpenTypeFeatureModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return d->currentFeatures.size();
}

int OpenTypeFeatureModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant OpenTypeFeatureModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const QString feature = d->currentFeatures.keys().at(index.row());
    KoOpenTypeFeatureInfo info = d->featureByTag(feature.toLatin1());
    if (role == Qt::DisplayRole) {
        return info.name;
    } else if (role == Qt::ToolTipRole) {
        return info.description;
    } else if (role == Qt::EditRole) {
        return d->currentFeatures.value(feature, QVariant(0)).toInt();
    } else if (role == Tag) {
        return feature;
    } else if (role == Sample) {
        return info.sample;
    } else if (role == Parameters) {
        QVariantList features;
        for (int i = 0; i <= info.maxValue; i++) {
            QVariantMap map;
            map.insert("value", QVariant(i));
            QString entry = i > 0? info.namedParameters.value(i-1): QString();
            if (entry.isEmpty()) {
                if (i == 1 && info.maxValue == 1) {
                    entry = info.name + ": " + i18nc("Feature value toggle", "On");
                } else if (i == 0) {
                    entry = info.name + ": " + i18nc("Feature value toggle", "Off");
                } else {
                    entry = info.name + ": " + QString::number(i);
                }
            }
            map.insert("display", entry);
            features.append(map);
        }
        return features;
    } else if (role == Max) {
        return info.maxValue;
    }
    return QVariant();
}

bool OpenTypeFeatureModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        // FIXME: Implement me!

        if (index.isValid() && role == Qt::EditRole) {
            const QString feature = d->currentFeatures.keys().at(index.row());
            d->currentFeatures.insert(feature, value.toInt());
            emit openTypeFeaturesChanged();
            emit dataChanged(index, index, {role});
            return true;
        }
        return false;
    }

    return false;
}

Qt::ItemFlags OpenTypeFeatureModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index) | Qt::ItemNeverHasChildren | Qt::ItemIsEditable;

    return flags;
}

QHash<int, QByteArray> OpenTypeFeatureModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[Tag] = "tag";
    roles[Sample] = "sample";
    roles[Parameters] = "parameters";
    roles[Max] = "max";
    return roles;
}

void OpenTypeFeatureModel::setFromTextProperties(const KoSvgTextProperties &props)
{
    const bool changeFont = props.cssFontInfo() != d->fontInfo;
    const QVariantMap newFeatures = props.propertyOrDefault(KoSvgTextProperties::FontFeatureSettingsId).toMap();
    const bool changeFeatures = d->currentFeatures != newFeatures;

    if (!changeFont && !changeFeatures) return;

    beginResetModel();
    if (changeFont) {
        const qreal res = 72.0;
        QVector<int> lengths;
        const KoCSSFontInfo info = props.cssFontInfo();
        const std::vector<FT_FaceSP> faces = KoFontRegistry::instance()->facesForCSSValues(
                    lengths,
                    info,
                    QString(),
                    static_cast<quint32>(res),
                    static_cast<quint32>(res));

        if (!faces.empty()) {
            QString language = props.propertyOrDefault(KoSvgTextProperties::TextLanguage).toString();
            // NOTE: We're retrieving only enough data for 6 samples here, to speed up loading.
            d->glyphModel->setFace(faces.front(), QLatin1String(language.toLatin1()), true);
            d->allFeatures->setAvailableFeatures(d->availableFeatures());
            d->fontInfo = props.cssFontInfo();
        }
    }

    if (changeFeatures) {
        d->currentFeatures = newFeatures;
        emit openTypeFeaturesChanged();
    }
    endResetModel();
}

QVariantMap OpenTypeFeatureModel::openTypeFeatures() const
{
    return d->currentFeatures;
}

void OpenTypeFeatureModel::setOpenTypeFeatures(const QVariantMap &newOpenTypeFeatures)
{
    if (d->currentFeatures == newOpenTypeFeatures)
        return;
    beginResetModel();
    d->currentFeatures = newOpenTypeFeatures;
    endResetModel();
    emit openTypeFeaturesChanged();
}

void OpenTypeFeatureModel::addFeature(const QString &tag)
{
    if (tag.isEmpty()) return;
    if (d->currentFeatures.keys().contains(tag)) return;

    QVariantMap dummy = d->currentFeatures;
    dummy.insert(tag, QVariant(1));
    const int index = dummy.keys().indexOf(tag);

    beginInsertRows(QModelIndex(), index, index);
    d->currentFeatures.insert(tag, QVariant(1));
    endInsertRows();
    emit openTypeFeaturesChanged();
}

void OpenTypeFeatureModel::removeFeature(const QString &tag)
{
    if (tag.isEmpty()) return;
    const int index = d->currentFeatures.keys().indexOf(tag);
    if (index < 0) return;

    beginRemoveRows(QModelIndex(), index, index);
    d->currentFeatures.remove(tag);
    endRemoveRows();
    emit openTypeFeaturesChanged();
}

QAbstractItemModel *OpenTypeFeatureModel::allFeatureModel() const
{
    return d->allFeatures;
}

void OpenTypeFeatureModel::setFromTextPropertiesModel(KoSvgTextPropertiesModel *textPropertiesModel)
{
    KoSvgTextProperties main;
    if (textPropertiesModel) {
        KoSvgTextPropertyData data = textPropertiesModel->textData.get();
        main = data.commonProperties;
        main.inheritFrom(data.inheritedProperties);
    }
    setFromTextProperties(main);
}

/********* AllOpenTypeFeaturesModel ************/
struct AllOpenTypeFeaturesModel::Private {

    KoOpenTypeFeatureInfoFactory factory;
    QList<KoOpenTypeFeatureInfo> availableFeatures; // Features in font.
    QStringList availableTags;
    QStringList allTags;

    bool featureAvailable(const QString &tag) const {
        return availableTags.contains(tag);
    }

    KoOpenTypeFeatureInfo featureByTag(QByteArray tag) const {
        Q_FOREACH(KoOpenTypeFeatureInfo feature, availableFeatures) {
            if (QLatin1String(feature.tag) == tag) {
                return feature;
                break;
            }
        }

        return factory.infoByTag(tag);
    }

    // OpenType allows for "custom" opentype features, this forloop checks for those.
    void setAvailableTags() {
        QStringList tags;
        allTags = factory.tags();
        Q_FOREACH(KoOpenTypeFeatureInfo feature, availableFeatures) {
            const QString tag = QString::fromLatin1(feature.tag.data(), 4);
            if (!allTags.contains(tag)) {
                allTags.append(tag);
            }
            tags.append(tag);
        }
        availableTags = tags;
    }
};

AllOpenTypeFeaturesModel::AllOpenTypeFeaturesModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new Private)
{

}

AllOpenTypeFeaturesModel::~AllOpenTypeFeaturesModel()
{

}

void AllOpenTypeFeaturesModel::setAvailableFeatures(const QList<KoOpenTypeFeatureInfo> &features)
{
    beginResetModel();
    d->availableFeatures = features;
    d->setAvailableTags();
    endResetModel();
}

QVariant AllOpenTypeFeaturesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const QString feature = d->allTags.at(index.row());
    KoOpenTypeFeatureInfo info = d->featureByTag(feature.toLatin1());
    if (role == Qt::DisplayRole) {
        return info.name;
    } else if (role == Qt::ToolTipRole) {
        return info.description;
    } else if (role == Tag) {
        return feature;
    } else if (role == Sample) {
        return info.sample;
    } else if (role == Available) {
        return d->featureAvailable(feature);
    }
    return QVariant();
}

int AllOpenTypeFeaturesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return d->allTags.size();
}

QVariant AllOpenTypeFeaturesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal && section == 0) {
            return i18nc("@title:column", "OpenType Feature Tag");
        }
    }
    return QVariant();
}

QHash<int, QByteArray> AllOpenTypeFeaturesModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[Tag] = "tag";
    roles[Sample] = "sample";
    roles[Available] = "available";
    return roles;
}

OpenTypeFeatureFilterModel::OpenTypeFeatureFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{

}

QString OpenTypeFeatureFilterModel::firstValidTag() const
{
    const QModelIndex idx = index(0, 0, QModelIndex());
    if (!idx.isValid()) return QString();
    return data(idx, AllOpenTypeFeaturesModel::Tag).toString();
}

#include <QDebug>
bool OpenTypeFeatureFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    if (!idx.isValid()) return false;
    const bool available = sourceModel()->data(idx, AllOpenTypeFeaturesModel::Available).toBool();
    if (m_filterAvailable && !available) return false;
    if (filterRegularExpression().pattern().isEmpty()) return true;
    const QString tag = sourceModel()->data(idx, AllOpenTypeFeaturesModel::Tag).toString();
    const QString name = sourceModel()->data(idx, Qt::DisplayRole).toString();
    return (tag.contains(filterRegularExpression()) || name.contains(filterRegularExpression()));
}

bool OpenTypeFeatureFilterModel::filterAvailable() const
{
    return m_filterAvailable;
}

void OpenTypeFeatureFilterModel::setFilterAvailable(bool newFilterAvailable)
{
    if (m_filterAvailable == newFilterAvailable)
        return;
    m_filterAvailable = newFilterAvailable;
    invalidateFilter();
    emit filterAvailableChanged();
}

QString OpenTypeFeatureFilterModel::searchText() const
{
    return filterRegularExpression().pattern();
}

void OpenTypeFeatureFilterModel::setSearchText(const QString &newSearchText)
{
    if (filterRegularExpression().pattern() == newSearchText)
        return;
    setFilterRegularExpression(newSearchText);
    emit searchTextChanged();
}
