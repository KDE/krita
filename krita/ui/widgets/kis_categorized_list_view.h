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

#ifndef KIS_CATEGORIZED_LIST_VIEW_H_
#define KIS_CATEGORIZED_LIST_VIEW_H_

#include <krita_export.h>
#include <QListView>

class QStyledItemDelegate;
class KoID;

class KRITAUI_EXPORT KisCategorizedListView: public QListView
{
    Q_OBJECT
public:
    KisCategorizedListView(bool useCheckBoxHack=false, QWidget* parent=0);
    virtual ~KisCategorizedListView();
    virtual void setModel(QAbstractItemModel* model);
    void updateRows(int begin, int end);

signals:
    void sigCategoryToggled(const QModelIndex& index, bool toggled);
    void sigEntryChecked(const QModelIndex& index);

protected slots:
    void slotIndexChanged(const QModelIndex& index);
    virtual void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    virtual void rowsInserted(const QModelIndex& parent, int start, int end);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);

private:
    bool m_useCheckBoxHack;
};


template<class TModel>
class KRITAUI_EXPORT KisCategorizedWidgetBase
{
public:
    KisCategorizedWidgetBase():
        m_model(0), m_delegate(0) { }

    int  indexOf(const KoID& entry)       const { return m_model->indexOf(entry).row();  }
    bool entryAt(KoID& result, int index) const { return m_model->entryAt(result,index); }

    const TModel* getModel() const { return m_model; }
    TModel*       getModel()       { return m_model; }

protected:
    TModel*              m_model;
    QStyledItemDelegate* m_delegate;
};

#endif // KIS_CATEGORIZED_LIST_VIEW_H_
