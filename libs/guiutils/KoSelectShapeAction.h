/* This file is part of the KDE libraries
    Copyright (C) 2008 Martin Pfeiffer <hubipete@gmx.net>

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

#ifndef KOSELECTSHAPEACTION_H
#define KOSELECTSHAPEACTION_H

#include <QWidgetAction>

#include "koguiutils_export.h"

/**
 */
class KOGUIUTILS_EXPORT KoSelectShapeAction : public QWidgetAction {
Q_OBJECT

public:
    KoSelectShapeAction( QObject* parent );

protected:
    QWidget* createWidget( QWidget* parent );

private slots:
    void addShape( QAction* action );

private:
    QMenu* createShapeMenu();
};

#endif // KOSELECTSHAPEACTION_H
