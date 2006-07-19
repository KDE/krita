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

#ifndef KO_ITEM_TOOLTIP_H
#define KO_ITEM_TOOLTIP_H

#include <QFrame>
#include <koffice_export.h>

class QStyleOptionViewItem;
class QModelIndex;
class QTextDocument;

class KOFFICEUI_EXPORT KoItemToolTip: public QFrame
{
    Q_OBJECT

    public:
        KoItemToolTip();
        virtual ~KoItemToolTip();
        void showTip( QWidget *widget, const QPoint &pos, const QStyleOptionViewItem &option, const QModelIndex &index );

    protected:
        virtual QTextDocument *createDocument( const QModelIndex &index ) = 0;

    private:
        typedef QFrame super;
        class Private;
        Private* const d;

        void updatePosition( QWidget *widget, const QPoint &pos, const QStyleOptionViewItem &option );

    public:
        virtual QSize sizeHint() const;

    protected:
        virtual void paintEvent( QPaintEvent *e );
        virtual void timerEvent( QTimerEvent *e );
        virtual bool eventFilter( QObject *object, QEvent *event );
};

#endif
