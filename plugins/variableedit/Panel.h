/* This file is part of the KDE project
 * Copyright (C) 2009 Jos van den Oever <jos@vandenoever.info>
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
#ifndef PANEL_H
#define PANEL_H

#include <KoCanvasObserver.h>

#include <QDockWidget>
#include <QHash>

#include <ui_Panel.h>

class Panel : public QDockWidget, public KoCanvasObserver
{
    Q_OBJECT
public:
    Panel(QWidget *parent = 0);
    ~Panel();

    virtual void setCanvas(KoCanvasBase *canvas);

private slots:
    void toolChangeDetected(const QString &toolId);
    void resourceChanged(int key, const QVariant &value);

private:
    KoCanvasBase *m_canvas;
    Ui::Panel widget;
};

#endif
