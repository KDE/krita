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
#include <QListView>
#include <QList>
#include <QMap>
#include <QString>
#include <utility>

enum
{
    IsHeaderRole       = Qt::UserRole + 1,
    ExpandCategoryRole = Qt::UserRole + 2
};

template<class T>
class KisCategorizedListModel: public QAbstractListModel
{
    struct Category
    {
        Category(const QString& n): name(n), expanded(true) { };
        bool operator < (const Category& c) const { return name < c.name; }
        bool operator == (const Category& c) const { return name == c.name; }
        int  size() const { return expanded ? (items.size() + 1) : 1; }
        
        QString  name;
        QList<T> items;
        bool     expanded;
    };
    
    typedef QList<Category>                       CategoryList;
    typedef typename CategoryList::iterator       Iterator;
    typedef typename CategoryList::const_iterator ConstIterator;
    typedef std::pair<qint32,qint32>              Index;
    
public:
    void addItem(const QString& category, const T& item)
    {
        Iterator itr = qFind(m_categories.begin(), m_categories.end(), category);
        
        if(itr != m_categories.end()) {
            itr->items.push_back(item);
        }
        else {
            m_categories.push_back(Category(category));
            m_categories.back().items.push_back(item);
        }
    }
    
    virtual int rowCount(const QModelIndex& parent) const {
        Q_UNUSED(parent);
        
        int numRows = 0;
        
        for(ConstIterator itr=m_categories.begin(); itr!=m_categories.end(); ++itr) {
            numRows += itr->size();
        }
        
        return numRows;
    }
        
    virtual QVariant data(const QModelIndex& idx, int role=Qt::DisplayRole) const {
        Index index = getIndex(idx);
        
        if(isValidIndex(index)) {
            if(isHeader(index)) {
                switch(role)
                {
                    case Qt::DisplayRole:    { return m_categories[index.first].name;     }
                    case IsHeaderRole:       { return true;                               }
                    case ExpandCategoryRole: { return m_categories[index.first].expanded; }
                }
            }
            else {
                switch(role)
                {
                    case Qt::DisplayRole: { return m_categories[index.first].items[index.second]; }
                    case IsHeaderRole:    { return false;                                         }
                }
            }
        }
        
        return QVariant();
    }
    
    virtual bool setData(const QModelIndex& idx, const QVariant& value, int role = Qt::EditRole) {
        Index index = getIndex(idx);
        
        if(isValidIndex(index)) {
            if(role == ExpandCategoryRole && isHeader(index)) {
                emit layoutAboutToBeChanged();
                m_categories[index.first].expanded = value.toBool();
                emit layoutChanged();
            }
        }
        
        return false;
    }
    
    virtual Qt::ItemFlags flags(const QModelIndex& idx) const {
        Index index = getIndex(idx);
        
        if(isHeader(index))
            return Qt::ItemIsEnabled;
        
        return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
    }
    
private:
    const Index getIndex(const QModelIndex& index) const {
        int    currRow       = 0;
        qint32 categoryIndex = 0;
        
        for(ConstIterator itr=m_categories.begin(); itr!=m_categories.end(); ++itr) {
            int endRow = currRow + itr->size();
            
            if(index.row() >= currRow && index.row() < endRow) 
                return Index(categoryIndex, index.row() - currRow - 1);
            
            currRow = endRow;
            ++categoryIndex;
        }
        
        return Index(-1,-1);
    }
    
    bool isValidIndex(const Index& index) const { return index.first >= 0;                     }
    bool isHeader    (const Index& index) const { return index.first >= 0 && index.second < 0; }
    
private:
    CategoryList m_categories;
};

class KRITAUI_EXPORT ListDelegate: public QStyledItemDelegate
{
public:
    ListDelegate(QAbstractListModel* model):
        m_model(model) { }
    
private:
    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    
private:
    QAbstractListModel* m_model;
};

class KRITAUI_EXPORT KisCategorizedListView: public QListView
{
    Q_OBJECT
    
public:
     KisCategorizedListView();
    ~KisCategorizedListView();
    
    template<class T>
    void setModel(KisCategorizedListModel<T>* model) {
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
};

#endif // KIS_CATEGORIZED_LIST_VIEW_H
