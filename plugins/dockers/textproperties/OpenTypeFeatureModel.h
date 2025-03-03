/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef OPENTYPEFEATUREMODEL_H
#define OPENTYPEFEATUREMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

class KoSvgTextProperties;
class KoOpenTypeFeatureInfo;
class KoSvgTextPropertiesModel;

/**
 * @brief The OpenTypeFeatureModel class
 * This model keeps track of the currently set font-feature-settings property on a given piece of text,
 * and allows editing it. It also keeps track of the available features of a given font.
 *
 * Each feature is internally represented as a tag plus a number.
 */
class OpenTypeFeatureModel : public QAbstractItemModel
{
    Q_OBJECT

    // Current opentype features.
    Q_PROPERTY(QVariantMap openTypeFeatures READ openTypeFeatures WRITE setOpenTypeFeatures NOTIFY openTypeFeaturesChanged)
public:
    OpenTypeFeatureModel(QObject *parent = nullptr);

    ~OpenTypeFeatureModel();

    enum Roles {
        Tag = Qt::UserRole + 1, ///< QString, opentype tag.
        Sample, ///< QString, the sample for this feature, may be empty.
        Parameters, ///< QVariantList, indices with names of the feature count.
        Max ///< int, max count, default is 1, but can be larger depending on the font.
    };
    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QHash<int, QByteArray> roleNames() const override;

    // Set the current model from a given set of text properties, if the font is different. Called by setFromTextPropertiesModel.
    void setFromTextProperties(const KoSvgTextProperties &props);

    // Current opentype features list. Map of tags with integer values.
    QVariantMap openTypeFeatures() const;
    void setOpenTypeFeatures(const QVariantMap &newOpenTypeFeatures);

    // Add a feature by tag and set its value to 1.
    Q_INVOKABLE void addFeature(const QString &tag);

    // Remove a feature by tag.
    Q_INVOKABLE void removeFeature(const QString &tag);

    /**
     * @brief featureFilterModel
     * @return return the OpenTypeFeatureFilterModel that allows
     * searching and sorting on all opentype features, whether available in the font
     * or part of the standard.
     */
    Q_INVOKABLE QAbstractItemModel *featureFilterModel() const;

    /**
     * @brief setFromTextPropertiesModel
     * Set the current glyph model font from the lager text properties model.
     * This allows us to use this class from qml, with little fuss.
     * @param newTextProperties -- the lager model to set from.
     */
    Q_INVOKABLE void setFromTextPropertiesModel(KoSvgTextPropertiesModel *textPropertiesModel);

Q_SIGNALS:
    void openTypeFeaturesChanged();

private:

    struct Private;
    const QScopedPointer<Private> d;
};

// Source model that keeps track of all available and possible opentype features.
class AllOpenTypeFeaturesModel: public QAbstractListModel {
public:
    AllOpenTypeFeaturesModel(QObject *parent = nullptr);

    /* @see OpenTypeFeatureModel::Roles */
    enum Roles {
        Tag = Qt::UserRole + 1,
        Sample,
        Available // bool, represents whether the current feature is also present in the font.
    };
    ~AllOpenTypeFeaturesModel();

    // Set the features for the currently available font.
    void setAvailableFeatures(const QList<KoOpenTypeFeatureInfo> &features);

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QHash<int, QByteArray> roleNames() const override;
private:

    struct Private;
    const QScopedPointer<Private> d;
};

#endif // OPENTYPEFEATUREMODEL_H
