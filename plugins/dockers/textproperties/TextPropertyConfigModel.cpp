/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <KSharedConfig>
#include <KConfigGroup>

#include "TextPropertyConfigModel.h"

struct TextPropertyConfigModel::TextPropertyData {
    TextPropertyConfigModel::VisibilityState visibilityState;
    TextPropertyConfigModel::PropertyType propertyType;
    bool collapsed;
    QStringList searchTerms;
    QString toolTip;
    QString title;
};

const QMap<TextPropertyConfigModel::VisibilityState, QString> visibilityConfigNames = {
    {TextPropertyConfigModel::FollowDefault, "followDefault"},
    {TextPropertyConfigModel::AlwaysVisible, "alwaysVisible"},
    {TextPropertyConfigModel::NeverVisible, "neverVisible"},
    {TextPropertyConfigModel::WhenRelevant, "whenRelevant"},
    {TextPropertyConfigModel::WhenSet, "whenSet"},
};

struct TextPropertyConfigModel::Private {
    QStringList propertyNames;
    QMap<QString, TextPropertyConfigModel::TextPropertyData> propertyData;
    TextPropertyConfigModel::VisibilityState defaultVisibilityState;
};

TextPropertyConfigModel::TextPropertyConfigModel(QObject *parent)
    : QAbstractListModel{parent}
    , d(new Private)
{
}

TextPropertyConfigModel::~TextPropertyConfigModel()
{
}

int TextPropertyConfigModel::defaultVisibilityState() const {
    return int(d->defaultVisibilityState);
}

void TextPropertyConfigModel::setDefaultVisibilityState(const int state) {
    if (state == int(d->defaultVisibilityState))
        return;
    d->defaultVisibilityState = VisibilityState(state);
    emit defaultVisibilityStateChanged();
}

int TextPropertyConfigModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return d->propertyNames.size();
}

QVariant TextPropertyConfigModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();
    if (index.row() < 0 || index.row() >= d->propertyNames.size()) return QVariant();

    const QString name = d->propertyNames.at(index.row());

    const TextPropertyData data = d->propertyData.value(name);
    if (role == Name) {
        return name;
    } else if (role == Qt::DisplayRole) {
        return data.title;
    } else if (role == Qt::ToolTipRole) {
        return data.toolTip;
    } else if (role == Visibility) {
        return data.visibilityState != FollowDefault?
                    int(data.visibilityState):
                    d->defaultVisibilityState != FollowDefault? int(d->defaultVisibilityState): int(WhenRelevant);
    } else if (role == Collapsed) {
        return data.collapsed;
    } else if (role == SearchTerms) {
        return data.searchTerms;
    } else if (role == Type) {
        return int (data.propertyType);
    }
    return QVariant();
}

bool TextPropertyConfigModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) return false;
    if (index.row() < 0 || index.row() >= d->propertyNames.size()) return false;

    if (role == Visibility) {
        const QString name = d->propertyNames.at(index.row());
        d->propertyData[name].visibilityState = VisibilityState(value.toInt());
        emit dataChanged(index, index, {role});
        return true;
    } else if (role == Collapsed) {
        const QString name = d->propertyNames.at(index.row());
        d->propertyData[name].collapsed = value.toBool();
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}

bool TextPropertyConfigModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    if (sourceParent != destinationParent) return false;
    beginMoveRows(sourceParent, sourceRow, sourceRow+count, destinationParent, destinationChild);
    for (int i = 0; i < count; i++) {
        d->propertyNames.move(sourceRow+i, destinationChild+i);
    }
    endMoveRows();
    return true;
}

QHash<int, QByteArray> TextPropertyConfigModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[Visibility] = "visibilityState";
    roles[SearchTerms] = "searchTerms";
    roles[Collapsed] = "collapsed";
    roles[Type] = "type";
    roles[Name] = "name";
    roles[Qt::DisplayRole] = "title";
    return roles;
}

void TextPropertyConfigModel::saveConfiguration()
{
    KConfigGroup configGroup = KSharedConfig::openConfig()->group("TextPropertiesDock");
    Q_FOREACH (const QString name, d->propertyNames) {
        const TextPropertyData data = d->propertyData.value(name);
        configGroup.writeEntry(name, visibilityConfigNames.value(data.visibilityState));
    }
}

void TextPropertyConfigModel::loadFromConfiguration() {
    KConfigGroup configGroup = KSharedConfig::openConfig()->group("TextPropertiesDock");
    Q_FOREACH (const QString name, d->propertyNames) {
        d->propertyData[name].visibilityState = visibilityConfigNames.key(configGroup.readEntry(name));
    }
}

void TextPropertyConfigModel::addProperty(const QString name, const int propertyType, const QString title, const QString toolTip, const QString searchTerms, const int visibilityState, const bool collapsed)
{
    if (d->propertyNames.contains(name)) return;
    beginInsertRows(QModelIndex(), d->propertyNames.size(), d->propertyNames.size());
    TextPropertyData data = d->propertyData.value(name, TextPropertyData());
    data.title = title;
    data.propertyType = PropertyType(propertyType);
    data.toolTip = toolTip;
    data.searchTerms = searchTerms.split(",");
    data.visibilityState = visibilityState < 0? FollowDefault: VisibilityState(visibilityState);
    data.collapsed = collapsed;
    d->propertyData.insert(name, data);
    d->propertyNames.append(name);
    endInsertRows();
}

TextPropertyConfigFilterModel::TextPropertyConfigFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent) {
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

bool TextPropertyConfigFilterModel::showParagraphProperties() const {
    return m_showParagraphProperties;
}

void TextPropertyConfigFilterModel::setShowParagraphProperties(const bool show) {
    if (m_showParagraphProperties == show) return;
    m_showParagraphProperties = show;
    invalidateFilter();
    emit showParagraphPropertiesChanged();
}

void TextPropertyConfigFilterModel::setProxySourceModel(QAbstractItemModel *model) {
    if (sourceModel() == model) return;
    setSourceModel(model);
    emit proxySourceModelChanged();
}

bool TextPropertyConfigFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    if (!idx.isValid()) return false;

    const QString name = sourceModel()->data(idx, Qt::DisplayRole).toString();
    const QStringList searchTerms = sourceModel()->data(idx, TextPropertyConfigModel::SearchTerms).toStringList();
    const TextPropertyConfigModel::PropertyType type = TextPropertyConfigModel::PropertyType(sourceModel()->data(idx, TextPropertyConfigModel::Type).toInt());
    if (type == TextPropertyConfigModel::Paragraph && !m_showParagraphProperties) {
        return false;
    } else if (type == TextPropertyConfigModel::Character && m_showParagraphProperties) {
        return false;
    }

    return (name.contains(filterRegularExpression()) || searchTerms.filter(filterRegularExpression()).size()>0);
}
