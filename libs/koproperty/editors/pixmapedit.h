/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004  Alexander Dymo <cloudtemple@mskat.net>
   Copyright (C) 2005-2008 Jarosław Staniek <staniek@kde.org>

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

#ifndef KPROPERTY_PIXMAPEDIT_H
#define KPROPERTY_PIXMAPEDIT_H

#include "koproperty/Factory.h"

#include <QtGui/QPixmap>
#include <QtCore/QVariant>

class QLabel;
class KPushButton;

namespace KoProperty
{

class KOPROPERTY_EXPORT PixmapEdit : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ value WRITE setValue USER true)

public:
    explicit PixmapEdit(Property *prop, QWidget *parent = 0);
    ~PixmapEdit();

    QVariant value() const;

public slots:
    void setValue(const QVariant &value);
//moved    virtual void drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value);

//    void resizeEvent(QResizeEvent *ev);
    bool eventFilter(QObject *o, QEvent *ev);

signals:
    void commitData( QWidget * editor );

protected:
//    virtual void setReadOnlyInternal(bool readOnly);

protected slots:
    /*! Helper used by selectPixmap(). Can be also used by subclassess.
     Selected path will be stored in "lastVisitedImagePath" config entry within "Recent Dirs"
     config group of application's settings. This entry can be later reused when file dialogs
     are opened for selecting image files. */
    QString selectPixmapFileName();

    /*! Selects a new pixmap using "open" file dialog. Can be reimplemented. */
    virtual void selectPixmap();

protected:
    QLabel *m_edit;
    QLabel *m_popup;
    KPushButton *m_button;
    Property *m_property;
//todo    QVariant m_recentlyPainted;
    QPixmap m_pixmap, /* todo? m_scaledPixmap,*/ m_previewPixmap;
};

class KOPROPERTY_EXPORT PixmapDelegate : public EditorCreatorInterface, 
                                         public ValuePainterInterface
{
public:
    PixmapDelegate();
    
    virtual QWidget * createEditor( int type, QWidget *parent, 
        const QStyleOptionViewItem & option, const QModelIndex & index ) const;

    virtual void paint( QPainter * painter, 
        const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

}

#endif
