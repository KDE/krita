/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef LIBKIS_KRITA_H
#define LIBKIS_KRITA_H

#include <QObject>
#include <QPointer>
#include <QAction>

#include "application.h"
#include "mainwindow.h"
#include "view.h"
#include "document.h"
#include "image.h"

#include <krita_export.h>

class DockWidgetFactoryBase;
class ViewExtension;

class LIBKIS_EXPORT Krita : public QObject
{
    Q_OBJECT
public:
    explicit Krita(QObject *parent = 0);

    QList<MainWindow*> mainWindows();
    QList<View*> views();
    QList<Document*> documents();
    QList<Image*> images();

    QAction *createAction(const QString &text);
    
    void addViewExtension(ViewExtension* _viewExtension);
    QList<ViewExtension*> viewExtensions();
    void addDockWidgetFactory(DockWidgetFactoryBase* _factory );
    
    static Krita* instance();
Q_SIGNALS:

public Q_SLOTS:

private:
    static Krita* s_instance;
    QList<ViewExtension*> m_viewExtensions;
};

#endif // LIBKIS_KRITA_H
