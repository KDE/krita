/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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
#ifndef KOVIEWCHILD_H
#define KOVIEWCHILD_H

#include <KoChild.h>

#include <QPointer>

class KoDocumentChild;
class KoView;
class KoFrame;

/**
 * This class represents an active embedded document.
 */
class KoViewChild : public KoChild
{
    Q_OBJECT
    public:
        KoViewChild( KoDocumentChild *child, KoView *_parentView );
        virtual ~KoViewChild();

        KoDocumentChild *documentChild() const { return m_child; }
        KoView *parentView() const { return m_parentView; }
        KoFrame *frame() const { return m_frame; }

        void setInitialFrameGeometry();

    public slots:

        // Call this when the view transformations change
        void reposition() { slotDocGeometryChanged(); }

    private slots:
        void slotFrameGeometryChanged();
        void slotDocGeometryChanged();

    private:
        QPointer<KoDocumentChild> m_child;
        QPointer<KoFrame> m_frame;
        QPointer<KoView> m_parentView;
        class KoViewChildPrivate;
        KoViewChildPrivate * const d;
};

#endif
