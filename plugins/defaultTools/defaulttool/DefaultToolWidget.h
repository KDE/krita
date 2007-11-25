/* This file is part of the KDE project
 * Copyright (C) 2007 Martin Pfeiffer <hubipete@gmx.net>
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
#ifndef DEFAULTTOOLWIDGET_H
#define DEFAULTTOOLWIDGET_H

#include <ui_DefaultToolWidget.h>
#include <KoFlake.h>

#include <QTabWidget>

class KoInteractionTool;
class QRadioButton;

class DefaultToolWidget : public QTabWidget, Ui::DefaultToolWidget {
    Q_OBJECT
public:
    explicit DefaultToolWidget( KoInteractionTool* tool, QWidget *parent = 0 );
/*
public slots:
    void updateControls();
*/
signals:
    /// emits whenever the hot (selected) position changes
    void hotPositionChanged( KoFlake::Position hotPosition );

protected:
    bool eventFilter( QObject *obj, QEvent *event );

private slots:
    void updatePosition();
    void positionHasChanged();
    void updateSize();
    void sizeHasChanged();
private:
    KoFlake::Position selectedPosition();
    QRadioButton * topLeft;
    QRadioButton * topRight;
    QRadioButton * bottomLeft;
    QRadioButton * bottomRight;
    QRadioButton * center;
    KoInteractionTool* m_tool;
};

#endif
