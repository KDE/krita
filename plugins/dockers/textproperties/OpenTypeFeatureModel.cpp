/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "OpenTypeFeatureModel.h"
#include <KoFontGlyphModel.h>
#include <KoCSSFontInfo.h>
#include <KoOpenTypeFeatureInfoFactory.h>
#include <KoFontLibraryResourceUtils.h>
#include <KoSvgTextProperties.h>
#include <KoFontRegistry.h>

struct OpenTypeFeatureModel::Private {
    Private(QObject *parent)
        : glyphModel(new KoFontGlyphModel(parent)) {
    }

    QList<KoOpenTypeFeatureInfo> availableFeatures() const {
        if (glyphModel) {
            return glyphModel->featureInfo().values();
        }
        return QList<KoOpenTypeFeatureInfo>();
    }

    KoOpenTypeFeatureInfo featureByTag(QLatin1String tag) const {
        KoOpenTypeFeatureInfo info = factory.infoByTag(tag);
        if (glyphModel) {
            info = glyphModel->featureInfo().value(tag, info);
        }
        return info;
    }

    KoFontGlyphModel *glyphModel {nullptr};
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
    KoOpenTypeFeatureInfo info = d->featureByTag(QLatin1String(feature.toLatin1()));
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
            d->glyphModel->setFace(faces.front(), QLatin1String(language.toLatin1()));
            emit availableFeaturesChanged();
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

QVariantList OpenTypeFeatureModel::availableFeatures() const
{
    QVariantList features;
    Q_FOREACH(const KoOpenTypeFeatureInfo info, d->availableFeatures()) {
        QVariantMap feat;

        feat.insert("tag", info.tag);
        feat.insert("display", info.name);
        feat.insert("toolTip", info.description);
        feat.insert("sample", info.sample);
        features.append(feat);
    }
    return features;
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
