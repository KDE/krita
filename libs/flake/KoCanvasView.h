/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOCANVASVIEW_H
#define KOCANVASVIEW_H

#include <koffice_export.h>

#include "KoCanvasBase.h"

#include <QWidget>
#include <QScrollArea>

class QGridLayout;
class QPaintEvent;

class FLAKE_EXPORT KoCanvasView : public QScrollArea {
public:
    KoCanvasView(QWidget *parent);
    virtual ~KoCanvasView() {};

    void setCanvas(KoCanvasBase *canvas);
    KoCanvasBase* canvas() const;

    int visibleHeight() const;
    int visibleWidth() const;
    int canvasOffsetX() const;
    int canvasOffsetY() const;

    void centerCanvas(bool centered);
    bool isCanvasCentered() const;

private:
    class Viewport : public QWidget {
      public:
        Viewport();
        ~Viewport() {};
        void setCanvas(QWidget *canvas);
        void centerCanvas(bool centered);
      private:
        QGridLayout *m_layout;
    };

private:
    KoCanvasBase *m_canvas;
    QWidget *m_canvasWidget;
    Viewport *m_viewport;
    bool m_centerCanvas;

};

#endif
