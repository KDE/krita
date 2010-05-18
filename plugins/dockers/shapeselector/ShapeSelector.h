/*
 * Copyright (C) 2006-2008 Thomas Zander <zander@kde.org>
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

#include <QDockWidget>

class FolderShape;
class Canvas;
class KUrl;
class QFile;

/**
 * The shape selector shows a widget that holds templates and clipboard data
 * for the user to easilly move that between apps and maintain functionality.
 */
class ShapeSelector : public QDockWidget
{
    Q_OBJECT
public:
    explicit ShapeSelector(QWidget *parent = 0);
    ~ShapeSelector();

    void addItems(const KUrl &url, FolderShape *targetFolder = 0);
    void addItems(QFile &file, FolderShape *targetFolder = 0);
    void setMainFolder(FolderShape *main);

private slots:
    void setSize(const QSize &size);
    void itemSelected();

private:
    Canvas *m_canvas;
};

#endif
