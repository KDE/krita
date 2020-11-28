/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_CATEGORIES_MAPPER_H
#define __KIS_CATEGORIES_MAPPER_H

#include <QObject>
#include <QScopedPointer>
#include <kritaui_export.h>


/**
 * Templated classes cannot inherit QObject, so the signal handling is
 * moved to a separate class
 */
class KRITAUI_EXPORT __CategoriesSignalsBase : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void rowChanged(int row);
    void beginInsertRow(int row);
    void endInsertRow();
    void beginRemoveRow(int row);
    void endRemoveRow();
};


template<class TEntry, class TEntryToQStringConverter>
class KisCategoriesMapper : public __CategoriesSignalsBase
{
public:

    class DataItem
    {
    public:
        DataItem(const QString &categoryName, KisCategoriesMapper *parent)
            : m_name(categoryName),
              m_category(0),
              m_expanded(false),
              m_enabled(true),
              m_checkable(false),
              m_checked(false),
              m_locked(false),
              m_lockable(false),
              m_toggled(false),
              m_parent(parent)
        {
        }

        DataItem(const TEntry &entry, DataItem *category, KisCategoriesMapper *parent)
            : m_data(new TEntry(entry)),
              m_category(category),
              m_expanded(false),
              m_enabled(true),
              m_checkable(false),
              m_checked(false),
              m_locked(false),
              m_lockable(false),
              m_toggled(false),
              m_parent(parent)
        {
            Q_ASSERT(category);

            TEntryToQStringConverter converter;
            m_name = converter(entry);
        }

        TEntry* data() const {
            return m_data.data();
        }

        QString name() const {
            return m_name;
        }

        bool isCategory() const {
            Q_ASSERT(static_cast<bool>(m_category) == static_cast<bool>(m_data));
            return !m_category;
        }

        DataItem* parentCategory() const {
            return m_category;
        }

        bool isExpanded() const {
            return m_expanded;
        }

        void setExpanded(bool value) {
            Q_ASSERT(isCategory());
            if (m_expanded == value) return;

            m_expanded = value;
            m_parent->notifyCategoryExpanded(this);
        }

        bool isEnabled() const {
            return m_enabled;
        }

        void setEnabled(bool value) {
            if (m_enabled == value) return;

            m_enabled = value;
            notifyItemChanged();
        }

        bool isCheckable() const {
            return m_checkable;
        }

        void setCheckable(bool value) {
            if (m_checkable == value) return;

            m_checkable = value;
            notifyItemChanged();
        }

        bool isChecked() const {
            return m_checked;
        }

        void setChecked(bool value) {
            if (m_checked == value) return;

            setToggled(value != m_checked);
            m_checked = value;
            notifyItemChanged();
        }
        bool isLocked() const {
            return m_locked;
        }
        void setLocked(bool value){
            m_locked = value;
        }
        bool isLockable() const {
            return m_lockable;
        }
        void setLockable(bool value){
            m_lockable = value;
        }
        bool isToggled() const {
            return m_toggled;
        }
        void setToggled(bool value){
            m_toggled = value;
        }

    private:
        void notifyItemChanged() {
            m_parent->notifyItemChanged(this);
        }

    private:
        QString m_name;
        QScopedPointer<TEntry> m_data;
        DataItem *m_category;

        bool m_expanded;
        bool m_enabled;
        bool m_checkable;
        bool m_checked;
        bool m_locked;
        bool m_lockable;
        bool m_toggled;
        KisCategoriesMapper *m_parent;
    };

public:
    KisCategoriesMapper() {}
    ~KisCategoriesMapper() override {
        qDeleteAll(m_items);
    }

    DataItem* addCategory(const QString &category) {
        if (fetchCategory(category)) return 0;
        DataItem *item = new DataItem(category, this);

        emit beginInsertRow(m_items.size());
        m_items.append(item);
        emit endInsertRow();
        return item;
    }

    void removeCategory(const QString &category) {
        QMutableListIterator<DataItem*> it(m_items);
        DataItem *categoryItem = 0;

        int row = 0;
        while(it.hasNext()) {
            DataItem *item = it.next();

            if (!item->isCategory() &&
                item->parentCategory()->name() == category) {

                emit beginRemoveRow(row);
                it.remove();
                delete item;
                emit endRemoveRow();
            } else {
                if (item->isCategory() && item->name() == category) {
                    Q_ASSERT(!categoryItem);
                    categoryItem = item;
                }
                row++;
            }
        }

        if (categoryItem) {
            int row = m_items.indexOf(categoryItem);
            emit beginRemoveRow(row);
            delete m_items.takeAt(row);
            emit endRemoveRow();
        }
    }

    DataItem* addEntry(const QString &category, const TEntry &entry) {
        DataItem *categoryItem = fetchCategory(category);
        if (!categoryItem) {
            categoryItem = addCategory(category);
        }
        DataItem *item = new DataItem(entry, categoryItem, this);

        emit beginInsertRow(m_items.size());
        m_items.append(item);
        emit endInsertRow();
        return item;
    }

    void removeEntry(const QString &category, const TEntry &entry) {
        DataItem *item = fetchEntry(category, entry);
        if (!item) return;

        int row = m_items.indexOf(item);
        emit beginRemoveRow(row);
        delete m_items.takeAt(row);
        emit endRemoveRow();
    }

    DataItem* fetchCategory(const QString &category) const {
        Q_FOREACH (DataItem *item, m_items) {
            if (item->isCategory() && item->name() == category) return item;
        }
        return 0;
    }

    DataItem* fetchEntry(const QString &category, const TEntry &entry) const {
        Q_FOREACH (DataItem *item, m_items) {
            if (!item->isCategory() &&
                *item->data() == entry &&
                item->parentCategory()->name() == category) return item;
        }
        return 0;
    }

    DataItem* fetchOneEntry(const TEntry &entry) const {
        Q_FOREACH (DataItem *item, m_items) {
            if (!item->isCategory() &&
                *item->data() == entry) return item;
        }
        return 0;
    }

    QVector<DataItem*> itemsForCategory(const QString &category) const {
        QVector<DataItem*> filteredItems;

        Q_FOREACH (DataItem *item, m_items) {
            if (!item->isCategory() &&
                item->parentCategory()->name() == category) {

                filteredItems.append(item);
            }
        }

        return filteredItems;
    }

    void expandAllCategories() {
        Q_FOREACH (DataItem *item, m_items) {
            if (item->isCategory()) {
                item->setExpanded(true);
            }
        }
    }

    DataItem* itemFromRow(int row) const {
        return m_items[row];
    }

    int rowFromItem(DataItem *item) const {
        return m_items.indexOf(item);
    }

    int rowCount() const {
        return m_items.size();
    }

private:
    void notifyItemChanged(DataItem *item) {
        emit rowChanged(m_items.indexOf(item));
    }

    void notifyCategoryExpanded(DataItem *categoryItem) {
        Q_ASSERT(categoryItem->isCategory());
        notifyItemChanged(categoryItem);

        Q_FOREACH (DataItem *item, m_items) {
            if (!item->isCategory() &&
                item->parentCategory() == categoryItem) {

                notifyItemChanged(item);
            }
        }
    }

protected:
    QList<DataItem*>& testingGetItems() {
        return m_items;
    }

private:
    QList<DataItem*> m_items;
};

#endif /* __KIS_CATEGORIES_MAPPER_H */
