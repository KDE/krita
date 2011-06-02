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

#ifndef KIS_CATEGORIZED_LIST_VIEW_H
#define KIS_CATEGORIZED_LIST_VIEW_H

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
    struct Category
    {
        Category(const TCategory& n):
            id(n), expanded(true) { };
        
        bool operator < (const Category& c) const { return id < c.id; }
        bool operator == (const Category& c) const { return id == c.id; }
        int  size() const { return entries.size() + 1; }
        
        TCategory     id;
        QList<TEntry> entries;
        bool          expanded;
        QBitArray     disabled;
    };
    
    typedef QList<Category>                       CategoryList;
    typedef typename CategoryList::iterator       Iterator;
    typedef typename CategoryList::const_iterator ConstIterator;
    typedef std::pair<qint32,qint32>              Index;
    
public:
    void fill(const QMap<TCategory,TEntry>& map) {
        typedef typename QList<TCategory>::const_iterator       ListItr;
        typedef typename QMap<TCategory,TEntry>::const_iterator MapItr;
        
        QList<TCategory> categories = map.uniqueKeys();
        
        for(ListItr cat=categories.begin(); cat!=categories.end(); ++cat) {
            qint32 num = map.count(*cat);
            MapItr beg = map.find(*cat);
            MapItr end = beg + num;
            
            m_categories.push_back(*cat);
            m_categories.last().disabled.resize(num);
            
            for(; beg!=end; ++beg)
                m_categories.last().entries.push_back(*beg);
        }
    }
    
    void addEntry(const TCategory& category, const TEntry& entry)
    {
        Iterator itr = qFind(m_categories.begin(), m_categories.end(), category);
        
        if(itr != m_categories.end()) {
            itr->items.push_back(entry);
        }
        else {
            m_categories.push_back(category);
            m_categories.back().items.push_back(entry);
        }
        
        itr->disabled.resize(itr->entries.size());
    }
    
    bool getEntry(TEntry& entry, int idx) const {
        Index index = getIndex(idx);
        
        if(isValidIndex(index) && !isHeader(index)) {
            entry = m_categories[index.first].entries[index.second];
            return true;
        }
        return false;
    }
    
    int getIndex(const TEntry& entry) const {
        typedef typename QList<TEntry>::const_iterator Itr;
        qint32 row = 0;
        
        for(ConstIterator cat=m_categories.begin(); cat!=m_categories.end(); ++cat) {
            Itr itr = qFind(cat->entries.begin(), cat->entries.end(), entry);
            
            if(itr != cat->entries.end())
                return row + std::distance(cat->entries.begin(), itr) + 1;
            
           row += cat->size();
        }
        return -1;
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
        
        if(isHeader(index)) {
            switch(role)
            {
            case Qt::DisplayRole:
                return categoryToString(m_categories[index.first].id);
            case IsHeaderRole:
                return true;
            case ExpandCategoryRole:
                return m_categories[index.first].expanded;
            }
        }
        else {
            switch(role)
            {
            case Qt::DisplayRole:
                return entryToString(m_categories[index.first].entries[index.second]);
            case IsHeaderRole:
                return false;
            }
        }
        
        return QVariant();
    }
    
    virtual bool setData(const QModelIndex& idx, const QVariant& value, int role = Qt::EditRole) {
        Index index = getIndex(idx.row());
        
        if(!isValidIndex(index))
            return false;
        
        if(role == ExpandCategoryRole && isHeader(index)) {
            emit layoutAboutToBeChanged();
            m_categories[index.first].expanded = value.toBool();
            emit layoutChanged();
        }
        
        return false;
    }
    
    virtual Qt::ItemFlags flags(const QModelIndex& idx) const {
        Index index = getIndex(idx.row());
        
        if(!isValidIndex(index))
            return 0;
        
        if(isHeader(index))
            return Qt::ItemIsEnabled;
        
        if(m_categories[index.first].disabled.testBit(index.second))
            return 0;
        
        return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
    }
    
    bool isValidIndex(const Index& index) const { return index.first >= 0;                     }
    bool isHeader(const Index& index)     const { return index.first >= 0 && index.second < 0; }
    
protected:
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

class KRITAUI_EXPORT KisCategorizedItemDelegate2: public QStyledItemDelegate
{
public:
    KisCategorizedItemDelegate2(QAbstractListModel* model);
    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    
private:
    QAbstractListModel* m_model;
    QIcon               m_errorIcon;
};
/*
class KRITAUI_EXPORT KisCategorizedListView: public QListView
{
    Q_OBJECT
    
public:
     KisCategorizedListView(QWidget* parent=0);
    ~KisCategorizedListView();
    
    template<class T, class U>
    void setModel(KisCategorizedListModel<T,U>* model) {
        delete m_delegate;
        m_delegate = new ListDelegate(model);
        QListView::setModel(model);
        QListView::setItemDelegate(m_delegate);
    }

private:
    virtual void mousePressEvent(QMouseEvent* event);
    
private:
    QStyledItemDelegate* m_delegate;
    bool                 m_ignoreCurrentChanged;
};//*/

#endif // KIS_CATEGORIZED_LIST_VIEW_H
