/* This file is part of the KDE project
   Copyright 2007 Montel Laurent <montel@kde.org>

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

#ifndef PICTURE_TOOL
#define PICTURE_TOOL

#include <KoTool.h>
#include <KJob>

class PictureShape;

class PictureTool : public KoTool
{
    Q_OBJECT
public:
    explicit PictureTool( KoCanvasBase* canvas );

    /// reimplemented from KoTool
    virtual void paint( QPainter& , const KoViewConverter& ) {}
    /// reimplemented from KoTool
    virtual void mousePressEvent( KoPointerEvent* ) {}
    /// reimplemented from superclass
    virtual void mouseDoubleClickEvent( KoPointerEvent *event );
    /// reimplemented from KoTool
    virtual void mouseMoveEvent( KoPointerEvent* ) {}
    /// reimplemented from KoTool
    virtual void mouseReleaseEvent( KoPointerEvent* ) {}

    /// reimplemented from KoTool
    virtual void activate (bool temporary=false);
    /// reimplemented from KoTool
    virtual void deactivate();

protected:
    /// reimplemented from KoTool
    virtual QWidget * createOptionWidget();

private slots:
    void changeUrlPressed();
    void setImageData(KJob *job);

private:
    PictureShape *m_pictureshape;
};

#endif 
