/* This file is part of the KDE project
   Copyright 2007 Boudewijn Rempt <boud@valdyas.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KRITA_SHAPE
#define KRITA_SHAPE

#include <QObject>

#include <KoShape.h>

class QString;
class KUrl;

#define KritaShapeId "KritaShape"

/**
   KritaShape is a flake shape that embeds a Krita image, that is, a
   color-managed, multi-layered raster image.

   XXX:

   * make it possible to resize the shape to the native size of the
     image.

   * create a couple of good tools

   * determine what to do with layers, filters etc -- how much should
     be editable in place?

   * allow creation of new images & editing of them
*/

class KritaShape : public QObject, public KoShape
{

    Q_OBJECT

public:
    /**
       Create a new KritaShape.

       @param url. If not empty, the shape will be created with the
       specified image in the url
       @param profileName the display profile
    */
    KritaShape(const KUrl& url, const QString & profileName);
    virtual ~KritaShape();

    virtual void paint( QPainter& painter, const KoViewConverter& converter );

    void setDisplayProfile( const QString & profileName );
    void importImage( const KUrl & url );

private slots:

    void slotLoadingFinished();

private:

    class Private;
    Private * m_d;

};


#endif // KRITA_SHAPE
