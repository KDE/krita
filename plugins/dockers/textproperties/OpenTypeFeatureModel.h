/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef OPENTYPEFEATUREMODEL_H
#define OPENTYPEFEATUREMODEL_H

#include <QAbstractItemModel>

class KoSvgTextProperties;

class OpenTypeFeatureModel : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY(QVariantMap openTypeFeatures READ openTypeFeatures WRITE setOpenTypeFeatures NOTIFY openTypeFeaturesChanged)
    Q_PROPERTY(QVariantList availableFeatures READ availableFeatures NOTIFY availableFeaturesChanged)

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

    void setFromTextProperties(const KoSvgTextProperties &props);

    QVariantMap openTypeFeatures() const;
    void setOpenTypeFeatures(const QVariantMap &newOpenTypeFeatures);

    QVariantList availableFeatures() const;

    Q_INVOKABLE void addFeature(const QString &tag);
    Q_INVOKABLE void removeFeature(const QString &tag);

Q_SIGNALS:
    void openTypeFeaturesChanged();
    void availableFeaturesChanged();

private:

    struct Private;
    const QScopedPointer<Private> d;
};

#endif // OPENTYPEFEATUREMODEL_H
