/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TEXTPROPERTYCONFIGMODEL_H
#define TEXTPROPERTYCONFIGMODEL_H

#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QObject>

// This model helps to keep track of the relevant properties.

class TextPropertyConfigModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int defaultVisibilityState READ defaultVisibilityState WRITE setDefaultVisibilityState NOTIFY defaultVisibilityStateChanged)
public:

    enum VisibilityState {
        FollowDefault = 0, ///< Follow the default property.
        WhenRelevant, ///< Show when either set or inherited.
        WhenSet, ///< Show only when set.
        AlwaysVisible, ///< Always show property.
        NeverVisible, ///< Never show property.
    };
    Q_ENUM(VisibilityState)

    enum PropertyType {
        Character, ///< This property can be applied on a character level.
        Paragraph, ///< This property only does something when applied to a paragraph.
        Mixed ///< This property can be in either.
    };
    Q_ENUM(PropertyType)

    enum Roles {
        Visibility = Qt::UserRole + 1,
        Collapsed,
        SearchTerms,
        Type,
        Name,
    };

    explicit TextPropertyConfigModel(QObject *parent = nullptr);
    ~TextPropertyConfigModel();

    int defaultVisibilityState() const;
    void setDefaultVisibilityState(const int state);

    // Call this after adding all the properties.
    Q_INVOKABLE void loadFromConfiguration();

    Q_INVOKABLE void addProperty(const QString name,
                     const int propertyType = 0,
                     const QString title = QString(),
                     const QString toolTip = QString(),
                     const QString searchTerms = QString(),
                     const int visibilityState = -1,
                     const bool collapsed = false);

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;
    QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void defaultVisibilityStateChanged();
private Q_SLOTS:
    void saveConfiguration();
private:

    struct Private;
    struct TextPropertyData;
    const QScopedPointer<Private> d;
};

class TextPropertyConfigFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(bool showParagraphProperties READ showParagraphProperties WRITE setShowParagraphProperties NOTIFY showParagraphPropertiesChanged)
    Q_PROPERTY(QAbstractItemModel *sourceModel READ sourceModel WRITE setProxySourceModel NOTIFY proxySourceModelChanged)
public:
    TextPropertyConfigFilterModel(QObject *parent = nullptr);
    // QSortFilterProxyModel interface

    bool showParagraphProperties() const;

    void setShowParagraphProperties(const bool show);

    void setProxySourceModel(QAbstractItemModel *model);

Q_SIGNALS:
    void showParagraphPropertiesChanged();
    void proxySourceModelChanged();
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    bool m_showParagraphProperties;
};

#endif // TEXTPROPERTYCONFIGMODEL_H
