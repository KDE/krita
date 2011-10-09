/*
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_CATEGORIZED_LIST_MODEL_H
#define KIS_CATEGORIZED_LIST_MODEL_H

#include <krita_export.h>
#include <QAbstractListModel>
#include <QStyledItemDelegate>
#include <QList>
#include <QIcon>
#include <QString>
#include <QBitArray>
#include <utility>
#include <iterator>

enum
{
    IsHeaderRole       = Qt::UserRole + 1,
    ExpandCategoryRole = Qt::UserRole + 2,
    CategoryBeginRole  = Qt::UserRole + 3,
    CategoryEndRole    = Qt::UserRole + 4
};

template<class TCategory, class TEntry>
class KisCategorizedListModel: public QAbstractListModel
{
protected:
    struct Entry
    {
        Entry(const TEntry& n):
            data(n), disabled(false), checkable(false), checked(false) { }

        bool operator <  (const Entry& c) const { return data <  c.data; }
        bool operator == (const Entry& c) const { return data == c.data; }

        TEntry data;
        bool   disabled;
        bool   checkable;
        bool   checked;
    };

    struct Category
    {
        Category(const TCategory& n):
            data(n), expanded(true) { };

        bool operator <  (const Category& c) const { return data <  c.data; }
        bool operator == (const Category& c) const { return data == c.data; }
        int  size() const { return entries.size() + 1; }

        TCategory    data;
        QList<Entry> entries;
        bool         expanded;
    };

    template<class TCompFunc>
    struct ComparatorAdapter
    {
        ComparatorAdapter(TCompFunc func): compareFunc(func) { }
        template<class T>
        bool operator()(const T& a, const T& b) { return compareFunc(a.data, b.data); }
        TCompFunc compareFunc;
    };

    typedef QList<Category>                       CategoryList;
    typedef typename CategoryList::iterator       Iterator;
    typedef typename CategoryList::const_iterator ConstIterator;
    typedef std::pair<qint32,qint32>              Index;

public:
    void clear() {
        emit beginResetModel();
        m_categories.clear();
        emit endResetModel();
    }

    QModelIndex firstIndex() const {
        return QAbstractListModel::index(0);
    }

    QModelIndex lastIndex() const {
        if(!m_categories.empty()) {
            int end = getCategoryBegin(m_categories.size()-1) + m_categories.last().entries.size() - 1;
            return QAbstractListModel::index(end);
        }
        return QAbstractListModel::index(0);
    }

    void expandCategory(const TCategory& category, bool expand) {
        Iterator itr = qFind(m_categories.begin(), m_categories.end(), category);
        if(itr != m_categories.end()) {
            itr->expanded = expand;
            emit dataChanged(firstIndex(), lastIndex());
        }
    }

    void expandAllCategories(bool expand) {
        for(Iterator cat=m_categories.begin(); cat!=m_categories.end(); ++cat)
            cat->expanded = expand;
        emit dataChanged(firstIndex(), lastIndex());
    }

    void addCategory(const TCategory& category, bool expandCategories=true) {
        Iterator itr = qFind(m_categories.begin(), m_categories.end(), category);
        if(itr == m_categories.end()) {
            m_categories.push_back(category);
            m_categories.last().expanded = expandCategories;
        }
    }

    void addEntry(const TCategory& category, const TEntry& entry, bool checkable=false) {
        Iterator itr = qFind(m_categories.begin(), m_categories.end(), category);

        if(itr == m_categories.end()) {
            m_categories.push_back(category);
            itr = --m_categories.end();
        }

        int pos = getCategoryBegin(itr) + itr->entries.size();

        emit beginInsertRows(QModelIndex(), pos, pos);
        itr->entries.push_back(entry);
        itr->entries.back().checkable = checkable;
        emit endInsertRows();
    }

    void removeEntry(const TCategory& category, const TEntry& entry) {
        Iterator itr = qFind(m_categories.begin(), m_categories.end(), category);

        if(itr != m_categories.end()) {
            typedef typename QList<Entry>::iterator EntryItr;
            EntryItr found = qFind(itr->entries.begin(), itr->entries.end(), entry);

            if(found != itr->entries.end()) {
                int pos = getCategoryBegin(itr) + std::distance(itr->entries.begin(), found);
                emit beginRemoveRows(QModelIndex(), pos, pos);
                itr->entries.erase(found, found+1);
                emit endRemoveRows();
            }
        }
    }

    void addEntries(const QMap<TCategory,TEntry>& map, bool expandCategories=false, bool checkable=false) {
        typedef typename QList<TCategory>::const_iterator       ListItr;
        typedef typename QMap<TCategory,TEntry>::const_iterator MapItr;

        QList<TCategory> categories = map.uniqueKeys();
        QModelIndex      beg        = lastIndex();

        for(ListItr cat=categories.begin(); cat!=categories.end(); ++cat) {
            MapItr   beg = map.find(*cat);
            MapItr   end = beg + map.count(*cat);
            Iterator itr = qFind(m_categories.begin(), m_categories.end(), *cat);

            if(itr == m_categories.end()) {
                m_categories.push_back(*cat);
                m_categories.last().expanded = expandCategories;
                itr = m_categories.end() - 1;
            }

            for(; beg!=end; ++beg) {
                itr->entries.push_back(*beg);
                itr->entries.last().checkable = checkable;
            }
        }

        emit dataChanged(beg, lastIndex());
    }

    void fill(const QMap<TCategory,TEntry>& map, bool expandCategories=false, bool checkable=false) {
        typedef typename QList<TCategory>::const_iterator       ListItr;
        typedef typename QMap<TCategory,TEntry>::const_iterator MapItr;

        m_categories.clear();
        QList<TCategory> categories = map.uniqueKeys();

        for(ListItr cat=categories.begin(); cat!=categories.end(); ++cat) {
            MapItr beg = map.find(*cat);
            MapItr end = beg + map.count(*cat);

            m_categories.push_back(*cat);
            m_categories.last().expanded = expandCategories;

            for(; beg!=end; ++beg) {
                m_categories.last().entries.push_back(*beg);
                m_categories.last().entries.last().checkable = checkable;
            }
        }

        emit dataChanged(firstIndex(), lastIndex());
    }

    void clearCategory(const TCategory& category) {
        Iterator itr = qFind(m_categories.begin(), m_categories.end(), category);

        if(itr != m_categories.end() && !itr->entries.empty()) {
            int pos = getCategoryBegin(itr);
            emit beginRemoveRows(QModelIndex(), pos, pos + itr->entries.size() - 1);
            itr->entries.clear();
            emit endRemoveRows();
        }
    }

    template<class TLessThan>
    void sortCategories(TLessThan comparator) {
        emit layoutAboutToBeChanged();
        qSort(m_categories.begin(), m_categories.end(), ComparatorAdapter<TLessThan>(comparator));
        emit layoutChanged();
    }

    template<class TLessThan>
    void sortEntries(TLessThan comparator) {
        emit layoutAboutToBeChanged();
        for(Iterator cat=m_categories.begin(); cat!=m_categories.end(); ++cat)
            qSort(cat->entries.begin(), cat->entries.end(), ComparatorAdapter<TLessThan>(comparator));
        emit layoutChanged();
    }

    void sortCategories() {
        emit layoutAboutToBeChanged();
        qSort(m_categories.begin(), m_categories.end());
        emit layoutChanged();
    }

    void sortEntries() {
        emit layoutAboutToBeChanged();
        for(Iterator cat=m_categories.begin(); cat!=m_categories.end(); ++cat)
            qSort(cat->entries.begin(), cat->entries.end());
        emit layoutChanged();
    }

    bool getCategory(QList<TEntry>& result, const TCategory& category) const {
        ConstIterator itr = qFind(m_categories.begin(), m_categories.end(), category);

        if(itr != m_categories.end()) {
            typedef typename QList<Entry>::const_iterator EntryItr;

            for(EntryItr i=itr->entries.begin(); i!=itr->entries.end(); ++i)
                result.push_back(i->data);

            return true;
        }

        return false;
    }

    bool entryAt(TEntry& entry, int idx) const {
        Index index = getIndex(idx);

        if(isValidIndex(index) && !isHeader(index)) {
            entry = m_categories[index.first].entries[index.second].data;
            return true;
        }
        return false;
    }

    QModelIndex indexOf(const TEntry& entry) const {
        typedef typename QList<Entry>::const_iterator Itr;
        qint32 row = 0;

        for(ConstIterator cat=m_categories.begin(); cat!=m_categories.end(); ++cat) {
            Itr itr = qFind(cat->entries.begin(), cat->entries.end(), entry);

            if(itr != cat->entries.end())
                return index(row + std::distance(cat->entries.begin(), itr) + 1);

            row += cat->size();
        }
        return index(-1);
    }

    virtual int rowCount(const QModelIndex& parent) const {
        Q_UNUSED(parent);

        int numRows = 0;

        for(ConstIterator itr=m_categories.begin(); itr!=m_categories.end(); ++itr)
            numRows += itr->size();

        return numRows;
    }

    virtual QVariant data(const QModelIndex& idx, int role=Qt::DisplayRole) const {
        Index index = getIndex(idx.row());

        if(!isValidIndex(index))
            return QVariant();

        switch(role)
        {
        case CategoryBeginRole:
            return getCategoryBegin(idx.row());
        case CategoryEndRole:
            return getCategoryBegin(idx.row()) + m_categories[index.first].entries.size();
        case ExpandCategoryRole:
            return m_categories[index.first].expanded;
        }

        if(isHeader(index)) {
            switch(role)
            {
            case Qt::ToolTipRole:
            case Qt::DisplayRole:
                return categoryToString(m_categories[index.first].data);
            case IsHeaderRole:
                return true;
            }
        }
        else {
            switch(role)
            {
            case Qt::ToolTipRole:
            case Qt::DisplayRole:
                return entryToString(m_categories[index.first].entries[index.second].data);
            case IsHeaderRole:
                return false;
            case Qt::CheckStateRole:
            {
                bool isCheckable = m_categories[index.first].entries[index.second].checkable;
                bool isChecked   = m_categories[index.first].entries[index.second].checked;

                if(isCheckable)
                    return isChecked ? Qt::Checked : Qt::Unchecked;
            }
                break;
            }
        }

        return QVariant();
    }

    virtual bool setData(const QModelIndex& idx, const QVariant& value, int role = Qt::EditRole) {
        if(!idx.isValid())
            return false;

        Index index = getIndex(idx.row());
        bool  isHdr = isHeader(index);

        if(role == ExpandCategoryRole && isHdr) {
            int beg = getCategoryBegin(idx.row());
            int end = beg - 1 + m_categories[index.first].entries.size();

            m_categories[index.first].expanded = value.toBool();
            emit dataChanged(QAbstractListModel::index(beg), QAbstractListModel::index(end));
        }
        else if(role == Qt::CheckStateRole && !isHdr) {
            if(m_categories[index.first].entries[index.second].checkable) {
                m_categories[index.first].entries[index.second].checked = bool(value.toInt() == Qt::Checked);
                emit dataChanged(idx, idx);
            }
        }

        return false;
    }

    virtual Qt::ItemFlags flags(const QModelIndex& idx) const {
        if(!idx.isValid())
            return 0;

        Index index = getIndex(idx.row());

        if(isHeader(index))
            return Qt::ItemIsEnabled;

        Qt::ItemFlags flags = 0;

        if(!m_categories[index.first].entries[index.second].disabled)
            flags |= Qt::ItemIsEnabled|Qt::ItemIsSelectable;
        if(m_categories[index.first].entries[index.second].checkable)
            flags |= Qt::ItemIsUserCheckable;

        return flags;
    }

protected:
    bool isValidIndex(const Index& index) const { return index.first >= 0;                     }
    bool isHeader(const Index& index)     const { return index.first >= 0 && index.second < 0; }

    const Index getIndex(int index) const {
        if(index >= 0) {
            int    currRow       = 0;
            qint32 categoryIndex = 0;

            for(ConstIterator itr=m_categories.begin(); itr!=m_categories.end(); ++itr) {
                int endRow = currRow + itr->size();

                if(index >= currRow && index < endRow)
                    return Index(categoryIndex, index - currRow - 1);

                currRow = endRow;
                ++categoryIndex;
            }
        }
        return Index(-1,-1);
    }

    int getCategoryBegin(int index) const {
        if(index >= 0) {
            int currRow = 0;

            for(ConstIterator itr=m_categories.begin(); itr!=m_categories.end(); ++itr) {
                int endRow = currRow + itr->size();

                if(index >= currRow && index < endRow)
                    return currRow + 1;

                currRow = endRow;
            }
        }
        return -1;
    }

    int getCategoryBegin(ConstIterator category) const {
        int currRow = 0;

        for(ConstIterator itr=m_categories.begin(); itr!=category; ++itr)
            currRow += itr->size();

        return currRow + 1;
    }

    virtual QString categoryToString(const TCategory& val) const = 0;
    virtual QString entryToString   (const TEntry&    val) const = 0;

protected:
    CategoryList m_categories;
};

#endif // KIS_CATEGORIZED_LIST_MODEL_H
