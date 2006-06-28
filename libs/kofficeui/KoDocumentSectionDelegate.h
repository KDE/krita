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

#ifndef KO_DOCUMENT_SECTION_DELEGATE_H
#define KO_DOCUMENT_SECTION_DELEGATE_H

#include <QAbstractItemDelegate>
#include <koffice_export.h>

class QAbstractItemView;
class KoDocumentSectionModel;

class KOFFICEUI_EXPORT KoDocumentSectionDelegate: public QAbstractItemDelegate
{
    typedef QAbstractItemDelegate super;
    Q_OBJECT

    public:
        KoDocumentSectionDelegate( QAbstractItemView *view, QObject *parent = 0 );
        virtual ~KoDocumentSectionDelegate();

        /// how items should be displayed
        enum DisplayMode
        {
            /// large fit-to-width thumbnails, with only titles or page numbers
            ThumbnailsMode,

            /// smaller thumbnails, with titles and property icons in two rows
            DetailedMode,

            /// no thumbnails, with titles and property icons in a single row
            MinimalMode
        };

        void setDisplayMode( DisplayMode mode );

        virtual void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
        virtual QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const;
        virtual bool editorEvent( QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index );

        virtual QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
        virtual void setEditorData( QWidget *editor, const QModelIndex &index ) const;
        virtual void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const;
        virtual void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex& index ) const;

    protected:
        virtual bool eventFilter( QObject *object, QEvent *event );

    private:
        typedef KoDocumentSectionModel Model;
        class Private;
        Private* const d;

        static QStyleOptionViewItem getOptions( const QStyleOptionViewItem &option, const QModelIndex &index );
        QRect textRect( const QStyleOptionViewItem &option, const QModelIndex &index ) const;
        QRect iconsRect( const QStyleOptionViewItem &option, const QModelIndex &index ) const;
        QRect thumbnailRect( const QStyleOptionViewItem &option, const QModelIndex &index ) const;
        void drawText( QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
        void drawIcons( QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
        void drawThumbnail( QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
};

#endif
