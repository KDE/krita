/*
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef SHAPESELECTOR_H
#define SHAPESELECTOR_H

#include "ZoomHandler.h"

#include <QDockWidget>

class Canvas;
class KoShape;
class KoShapeManager;
class FolderShape;

/**
 * The shape selector shows a widget that holds templates and clipboard data
 * for the user to easilly move that between apps and maintain functionality.
 */
class ShapeSelector : public QDockWidget {
    Q_OBJECT
public:
    explicit ShapeSelector(QWidget *parent = 0);
    ~ShapeSelector();

private slots:
    void loadShapeTypes();
    void setSize(const QSize &size);

private:
    void itemSelected();
    void add(KoShape *item);

private:
    friend class Canvas;

    KoShapeManager *m_shapeManager;
    Canvas *m_canvas;
    FolderShape *m_mainFolder;
};

#endif
