/*
  Copyright (c) 2006 Gábor Lehel <illissius@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef KIS_DOCUMENT_SECTION_DELEGATE_H
#define KIS_DOCUMENT_SECTION_DELEGATE_H

#include <QAbstractItemDelegate>
class KisNodeView;

class KisNodeModel;

/**
 * See KisNodeModel and KisNodeView.
 * 
 * A delegate provides the gui machinery, using Qt's model/view terminology.
 * This class is owned by KisNodeView to do the work of generating the 
 * graphical representation of each item.
 */
class KisNodeDelegate: public QAbstractItemDelegate
{
    Q_OBJECT

public:
    explicit KisNodeDelegate(KisNodeView *view, QObject *parent = 0);
    virtual ~KisNodeDelegate();

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex& index) const;

protected:
    virtual bool eventFilter(QObject *object, QEvent *event);


private:
    typedef KisNodeModel Model;
    typedef KisNodeView View;
    class Private;
    Private* const d;

    static QStyleOptionViewItemV4 getOptions(const QStyleOptionViewItem &option, const QModelIndex &index);
    QRect progressBarRect(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawProgressBar(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void drawColorLabel(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawFrame(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawThumbnail(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QRect iconsRect(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QRect textRect(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawText(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawIcons(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QRect visibilityClickRect(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawVisibilityIconHijack(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawDecoration(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawExpandButton(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private Q_SLOTS:
    void slotOnCloseEditor();
};

#endif
