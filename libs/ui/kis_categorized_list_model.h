/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_CATEGORIZED_LIST_MODEL_H
#define __KIS_CATEGORIZED_LIST_MODEL_H

#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include "kis_categories_mapper.h"

class KRITAUI_EXPORT __CategorizedListModelBase : public QAbstractListModel
{
    Q_OBJECT

public:
    enum AdditionalRoles {
        IsHeaderRole       = Qt::UserRole + 1,
        ExpandCategoryRole = Qt::UserRole + 2,
        SortRole           = Qt::UserRole + 3,
        isLockedRole       = Qt::UserRole + 4,
        isLockableRole     = Qt::UserRole + 5,
        isToggledRole      = Qt::UserRole + 6
    };

public:
    __CategorizedListModelBase(QObject *parent);
    ~__CategorizedListModelBase() override;

private Q_SLOTS:

    void slotRowChanged(int row) {
        QModelIndex changedIndex(index(row));
        emit dataChanged(changedIndex, changedIndex);
    }

    void slotBeginInsertRow(int row) {
        beginInsertRows(QModelIndex(), row, row);
    }

    void slotEndInsertRow() {
        endInsertRows();
    }

    void slotBeginRemoveRow(int row) {
        beginRemoveRows(QModelIndex(), row, row);
    }

    void slotEndRemoveRow() {
        endRemoveRows();
    }
};

template<class TEntry, class TEntryToQStringConverter>
class KisCategorizedListModel : public __CategorizedListModelBase
{
public:
    typedef TEntry Entry_Type;
    typedef KisCategoriesMapper<TEntry, TEntryToQStringConverter> SpecificCategoriesMapper;
    typedef typename SpecificCategoriesMapper::DataItem DataItem;

public:
    KisCategorizedListModel(QObject *parent = 0)
        : __CategorizedListModelBase(parent)
    {
        connect(&m_mapper, SIGNAL(rowChanged(int)), SLOT(slotRowChanged(int))); // helps with category expand menu
        connect(&m_mapper, SIGNAL(beginInsertRow(int)), SLOT(slotBeginInsertRow(int)));
        connect(&m_mapper, SIGNAL(endInsertRow()), SLOT(slotEndInsertRow()));
        connect(&m_mapper, SIGNAL(beginRemoveRow(int)), SLOT(slotBeginRemoveRow(int)));
        connect(&m_mapper, SIGNAL(endRemoveRow()), SLOT(slotEndRemoveRow()));



    }

    int rowCount(const QModelIndex& parent) const override {
        Q_UNUSED(parent);
        return m_mapper.rowCount();
    }

    QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override {
        if (!idx.isValid()) return QVariant();

        typename SpecificCategoriesMapper::DataItem *item =
            m_mapper.itemFromRow(idx.row());
        Q_ASSERT(item);

        switch (role) {
        case IsHeaderRole:
            return item->isCategory();
        case ExpandCategoryRole:
            return item->isCategory() ? item->isExpanded() : item->parentCategory()->isExpanded();
        case Qt::ToolTipRole:
        case Qt::DisplayRole:
            return item->name();
        case Qt::CheckStateRole:
            return item->isCheckable() ? item->isChecked() ? Qt::Checked : Qt::Unchecked : QVariant();
        case SortRole:
            return item->isCategory() ? item->name() : item->parentCategory()->name() + item->name();
        case isLockedRole:
            return item->isLocked();
        case isLockableRole:
            return item->isLockable();
        case isToggledRole:
            return item->isToggled();

        }

        return QVariant();
    }

    bool setData(const QModelIndex& idx, const QVariant& value, int role = Qt::EditRole) override {
        if (!idx.isValid()) return false;

        typename SpecificCategoriesMapper::DataItem *item =
            m_mapper.itemFromRow(idx.row());
        Q_ASSERT(item);

        switch (role) {
        case ExpandCategoryRole:
            Q_ASSERT(item->isCategory());
            item->setExpanded(value.toBool());
            break;
        case Qt::CheckStateRole:
            Q_ASSERT(item->isCheckable());
            item->setChecked(value.toInt() == Qt::Checked);            
            break;        
        }

        // dataChanged() needs a QVector even though we are just passing one
        QVector<int> roles;
        roles.append(role);

        emit dataChanged(idx, idx, roles);
        return true;
    }

    Qt::ItemFlags flags(const QModelIndex& idx) const override {
        if (!idx.isValid()) return Qt::NoItemFlags;

        typename SpecificCategoriesMapper::DataItem *item =
            m_mapper.itemFromRow(idx.row());
        Q_ASSERT(item);

        Qt::ItemFlags flags = Qt::NoItemFlags;

        if (item->isEnabled()) {
            flags |= Qt::ItemIsEnabled;
        }

        if (!item->isCategory()) {
            flags |= Qt::ItemIsSelectable;

            if (item->isCheckable()) {
                flags |= Qt::ItemIsUserCheckable;
            }
        }

        return flags;
    }

    QModelIndex indexOf(const TEntry& entry) const {
        typename SpecificCategoriesMapper::DataItem *item =
            m_mapper.fetchOneEntry(entry);

        return index(m_mapper.rowFromItem(item));
    }

    bool entryAt(TEntry& entry, QModelIndex index) const {
        int row = index.row();
        if (row < 0 || row >= m_mapper.rowCount()) return false;

        typename SpecificCategoriesMapper::DataItem *item =
            m_mapper.itemFromRow(row);

        if (!item->isCategory()) {
            entry = *item->data();
            return true;
        }

        return false;
    }

    SpecificCategoriesMapper* categoriesMapper() {
        return &m_mapper;
    }

    const SpecificCategoriesMapper* categoriesMapper() const {
        return &m_mapper;
    }

private:
    SpecificCategoriesMapper m_mapper;
};

template<class TModel>
class KRITAUI_EXPORT KisSortedCategorizedListModel : public QSortFilterProxyModel
{
    typedef typename TModel::Entry_Type Entry_Type;

public:

    KisSortedCategorizedListModel(QObject *parent)
        : QSortFilterProxyModel(parent)
    {
    }

    QModelIndex indexOf(const Entry_Type& entry) const {
        /**
         * We don't use the source model's indexOf(), because
         * the items might be duplicated and we need to return the
         * topmost one in the sorted order.
         */

        Entry_Type e;

        for (int i = 0; i < rowCount(); i++) {
            QModelIndex index = this->index(i, 0);

            if (entryAt(e, index) && e == entry) {
                return index;
            }
        }

        return QModelIndex();
    }

    bool entryAt(Entry_Type &entry, QModelIndex index) const {
        QModelIndex srcIndex = mapToSource(index);
        return m_model->entryAt(entry, srcIndex);
    }

protected:
    void initializeModel(TModel *model) {
        m_model = model;
        setSourceModel(model);
        setSortRole(TModel::SortRole);
    }

    bool lessThanPriority(const QModelIndex &left,
                          const QModelIndex &right,
                          const QString &priorityCategory) const {

        QString leftKey = sourceModel()->data(left, sortRole()).toString();
        QString rightKey = sourceModel()->data(right, sortRole()).toString();

        bool leftIsSpecial = leftKey.startsWith(priorityCategory);
        bool rightIsSpecial = rightKey.startsWith(priorityCategory);

        return leftIsSpecial != rightIsSpecial ?
            leftIsSpecial : leftKey < rightKey;
    }

private:
    TModel *m_model;
};

#endif /* __KIS_CATEGORIZED_LIST_MODEL_H */
