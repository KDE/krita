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

#ifndef KO_DOCUMENT_SECTION_VIEW_H
#define KO_DOCUMENT_SECTION_VIEW_H

#include <QTreeView>
#include <koffice_export.h>

class KoDocumentSectionModel;

class KOFFICEUI_EXPORT KoDocumentSectionView: public QTreeView
{
    typedef QTreeView super;
    Q_OBJECT

    public:
        KoDocumentSectionView( QWidget *parent = 0 );
        virtual ~KoDocumentSectionView();

    protected:
        virtual bool event( QEvent *event );
        virtual bool viewportEvent( QEvent *event );

    protected slots:
        virtual void currentChanged( const QModelIndex &current, const QModelIndex &previous );
        virtual void dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight );

    private:
        typedef KoDocumentSectionModel Model;
        class Private;
        Private* const d;
};

#endif
