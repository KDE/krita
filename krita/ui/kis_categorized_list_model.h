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
    CategoryEndRole    = Qt::UserRole + 4,
};

template<class TCategory, class TEntry>
class KisCategorizedListModel: public QAbstractListModel
{
protected:
    struct Entry
    {
        Entry(const TEntry& n):
            data(n), disabled(false) { }
        
        bool operator <  (const Entry& c) const { return data <  c.data; }
        bool operator == (const Entry& c) const { return data == c.data; }
        
        TEntry data;
        bool   disabled;
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
    void clear() { m_categories.clear(); }
    
    void addCategory(const TCategory& category) {
        Iterator itr = qFind(m_categories.begin(), m_categories.end(), category);
        
        if(itr == m_categories.end())
            m_categories.push_back(category);
    }
    
    void addEntry(const TCategory& category, const TEntry& entry)
    {
        Iterator itr = qFind(m_categories.begin(), m_categories.end(), category);
        
        if(itr != m_categories.end()) {
            itr->entries.push_back(entry);
        }
        else {
            m_categories.push_back(category);
            m_categories.back().entries.push_back(entry);
        }
    }
    
    void addEntries(const QMap<TCategory,TEntry>& map) {
        typedef typename QList<TCategory>::const_iterator       ListItr;
        typedef typename QMap<TCategory,TEntry>::const_iterator MapItr;
        
        QList<TCategory> categories = map.uniqueKeys();
        
        for(ListItr cat=categories.begin(); cat!=categories.end(); ++cat) {
            MapItr   beg = map.find(*cat);
            MapItr   end = beg + map.count(*cat);
            Iterator itr = qFind(m_categories.begin(), m_categories.end(), *cat);
            
            if(itr == m_categories.end()) {
                m_categories.push_back(*cat);
                itr = m_categories.end() - 1;
            }
            
            for(; beg!=end; ++beg)
                itr->entries.push_back(*beg);
        }
    }
    
    void fill(const QMap<TCategory,TEntry>& map) {
        typedef typename QList<TCategory>::const_iterator       ListItr;
        typedef typename QMap<TCategory,TEntry>::const_iterator MapItr;
        
        clear();
        QList<TCategory> categories = map.uniqueKeys();
        
        for(ListItr cat=categories.begin(); cat!=categories.end(); ++cat) {
            MapItr beg = map.find(*cat);
            MapItr end = beg + map.count(*cat);
            
            m_categories.push_back(*cat);
            
            for(; beg!=end; ++beg)
                m_categories.last().entries.push_back(*beg);
        }
    }
    
    template<class TLessThan>
    void sortCategories(TLessThan comparator) {
        qSort(m_categories.begin(), m_categories.end(), ComparatorAdapter<TLessThan>(comparator));
    }
    
    template<class TLessThan>
    void sortEntries(TLessThan comparator) {
        for(Iterator cat=m_categories.begin(); cat!=m_categories.end(); ++cat)
            qSort(cat->entries.begin(), cat->entries.end(), ComparatorAdapter<TLessThan>(comparator));
    }
    
    void sortCategories() {
        qSort(m_categories.begin(), m_categories.end());
    }
    
    void sortEntries() {
        for(Iterator cat=m_categories.begin(); cat!=m_categories.end(); ++cat)
            qSort(cat->entries.begin(), cat->entries.end());
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
        
        if(role == CategoryBeginRole)
            return getCategoryBegin(idx.row());
        
        if(role == CategoryEndRole)
            return getCategoryBegin(idx.row()) + m_categories[index.first].entries.size();
        
        if(role == ExpandCategoryRole)
            return m_categories[index.first].expanded;
        
        if(isHeader(index)) {
            switch(role)
            {
            case Qt::DisplayRole:
                return categoryToString(m_categories[index.first].data);
            case IsHeaderRole:
                return true;
            }
        }
        else {
            switch(role)
            {
            case Qt::DisplayRole:
                return entryToString(m_categories[index.first].entries[index.second].data);
            case IsHeaderRole:
                return false;
            }
        }
        
        return QVariant();
    }
    
    virtual bool setData(const QModelIndex& idx, const QVariant& value, int role = Qt::EditRole) {
        if(!idx.isValid())
            return false;
        
        Index index = getIndex(idx.row());
        
        if(role == ExpandCategoryRole && isHeader(index)) {
            int beg = getCategoryBegin(idx.row());
            int end = beg - 1 + m_categories[index.first].entries.size();
            
            m_categories[index.first].expanded = value.toBool();
            emit dataChanged(QAbstractListModel::index(beg), QAbstractListModel::index(end));
        }
        
        return false;
    }
    
    virtual Qt::ItemFlags flags(const QModelIndex& idx) const {
        if(!idx.isValid())
            return 0;
        
        Index index = getIndex(idx.row());
        
        if(isHeader(index))
            return Qt::ItemIsEnabled;
        
        if(m_categories[index.first].entries[index.second].disabled)
            return 0;
        
        return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
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
    
    virtual QString categoryToString(const TCategory& val) const = 0;
    virtual QString entryToString   (const TEntry&    val) const = 0;
    
protected:
    CategoryList m_categories;
};

#endif // KIS_CATEGORIZED_LIST_MODEL_H
