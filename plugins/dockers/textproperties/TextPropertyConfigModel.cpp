/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <KSharedConfig>
#include <KConfigGroup>

#include "TextPropertyConfigModel.h"

struct TextPropertyConfigModel::TextPropertyData {
    TextPropertyConfigModel::VisibilityState visibilityState = TextPropertyConfigModel::FollowDefault;
    TextPropertyConfigModel::PropertyType propertyType = TextPropertyConfigModel::Mixed;
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

const QString DEFAULT_VISIBILITY_STATE = "defaultVisibilityState";

struct TextPropertyConfigModel::Private {
    QStringList propertyNames;
    QMap<QString, TextPropertyConfigModel::TextPropertyData> propertyData;
    TextPropertyConfigModel::VisibilityState defaultVisibilityState = TextPropertyConfigModel::WhenRelevant;
};

TextPropertyConfigModel::TextPropertyConfigModel(QObject *parent)
    : QAbstractListModel{parent}
    , d(new Private)
{
}

TextPropertyConfigModel::~TextPropertyConfigModel()
{
}

TextPropertyConfigModel::VisibilityState TextPropertyConfigModel::defaultVisibilityState() const {
    return d->defaultVisibilityState;
}

void TextPropertyConfigModel::setDefaultVisibilityState(const VisibilityState state) {
    if (state == d->defaultVisibilityState)
        return;
    d->defaultVisibilityState = state;
    emit defaultVisibilityStateChanged();
    emit shouldFilterChanged();
}

bool TextPropertyConfigModel::shouldFilter() const
{
    bool shouldFilter = d->defaultVisibilityState == AlwaysVisible;
    if (!shouldFilter) return false;

    for (int i = 0; i < d->propertyData.size(); i++) {
        VisibilityState state = d->propertyData.values().at(i).visibilityState;
        if (state == WhenRelevant || state == WhenSet) {
            return false;
            break;
        }
    }
    return shouldFilter;
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
        return data.visibilityState;
    } else if (role == SearchTerms) {
        return data.searchTerms;
    } else if (role == Type) {
        return int (data.propertyType);
    }
    return QVariant();
}

bool TextPropertyConfigModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || !value.isValid()) return false;
    if (index.row() < 0 || index.row() >= d->propertyNames.size()) return false;

    if (role == Visibility) {
        VisibilityState newState = VisibilityState(value.toInt());
        const QString name = d->propertyNames.at(index.row());
        if (d->propertyData[name].visibilityState != newState) {
            d->propertyData[name].visibilityState = VisibilityState(value.toInt());
            emit dataChanged(index, index, {role});
            emit shouldFilterChanged();
            return true;
        }
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
    roles[Visibility] = "visibility";
    roles[SearchTerms] = "searchTerms";
    roles[Type] = "type";
    roles[Name] = "name";
    roles[Qt::DisplayRole] = "title";
    return roles;
}

void TextPropertyConfigModel::saveConfiguration()
{
    KConfigGroup configGroup = KSharedConfig::openConfig()->group("TextProperties");
    configGroup.writeEntry(DEFAULT_VISIBILITY_STATE, visibilityConfigNames.value(d->defaultVisibilityState));
    Q_FOREACH (const QString name, d->propertyNames) {
        const TextPropertyData data = d->propertyData.value(name);
        configGroup.writeEntry(name, visibilityConfigNames.value(data.visibilityState));
    }
}

void TextPropertyConfigModel::loadFromConfiguration() {
    KConfigGroup configGroup = KSharedConfig::openConfig()->group("TextProperties");
    d->defaultVisibilityState = visibilityConfigNames.key(configGroup.readEntry(DEFAULT_VISIBILITY_STATE, visibilityConfigNames.value(d->defaultVisibilityState)));
    Q_FOREACH (const QString name, d->propertyNames) {
        d->propertyData[name].visibilityState = visibilityConfigNames.key(configGroup.readEntry(name, visibilityConfigNames.value(d->propertyData[name].visibilityState)));
    }
}

void TextPropertyConfigModel::addProperty(const QString &name, const int propertyType, const QString title, const QString toolTip, const QString searchTerms, const int visibilityState)
{
    if (d->propertyNames.contains(name)) return;
    beginInsertRows(QModelIndex(), d->propertyNames.size(), d->propertyNames.size());
    TextPropertyData data = d->propertyData.value(name, TextPropertyData());

    data.title = title;
    data.propertyType = PropertyType(propertyType);
    data.toolTip = toolTip;
    data.searchTerms = searchTerms.split(",");
    data.visibilityState = VisibilityState(qMax(visibilityState, 0));

    d->propertyData.insert(name, data);
    d->propertyNames.append(name);
    endInsertRows();
}

int TextPropertyConfigModel::visibilityStateForName(const QString &name) const
{
    TextPropertyData data = d->propertyData.value(name, TextPropertyData());
    return int(data.visibilityState);
}

TextPropertyConfigFilterModel::TextPropertyConfigFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent) {

    connect(this, &QAbstractItemModel::rowsRemoved, this, &TextPropertyConfigFilterModel::filteredNamesChanged);
    connect(this, &QAbstractItemModel::rowsInserted, this, &TextPropertyConfigFilterModel::filteredNamesChanged);
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

QStringList TextPropertyConfigFilterModel::filteredNames() const
{
    QStringList names;

    for (int i = 0; i < rowCount(); i++) {
        QModelIndex idx = index(i, 0);
        names.append(data(idx, TextPropertyConfigModel::Name).toString());
    }
    return names;
}

bool TextPropertyConfigFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    if (!idx.isValid()) return false;

    const TextPropertyConfigModel::VisibilityState state = TextPropertyConfigModel::VisibilityState(sourceModel()->data(idx, TextPropertyConfigModel::Visibility).toInt());
    if (state == TextPropertyConfigModel::NeverVisible) return false;

    const TextPropertyConfigModel::PropertyType type = TextPropertyConfigModel::PropertyType(sourceModel()->data(idx, TextPropertyConfigModel::Type).toInt());
    if (type == TextPropertyConfigModel::Paragraph && !m_showParagraphProperties) {
        return false;
    }

    const QString name = sourceModel()->data(idx, Qt::DisplayRole).toString();
    const QStringList searchTerms = sourceModel()->data(idx, TextPropertyConfigModel::SearchTerms).toStringList();

    return (name.contains(filterRegularExpression()) || searchTerms.filter(filterRegularExpression()).size()>0);
}
