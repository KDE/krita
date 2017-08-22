/*
    Copyright (C) 2012  Dan Leinir Turthra Jensen <admin@leinir.dk>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef TOOLMANAGER_H
#define TOOLMANAGER_H

#include <QQuickItem>

class KoCanvasController;

class ToolManager : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QObject* view READ view WRITE setView NOTIFY viewChanged)
    Q_PROPERTY(QObject* currentTool READ currentTool NOTIFY currentToolChanged);
public:
    explicit ToolManager(QQuickItem* parent = 0);
    virtual ~ToolManager();

    QObject* view() const;
    void setView(QObject* newView);
    QObject* currentTool() const;

    Q_INVOKABLE void requestToolChange(QString toolID);

public Q_SLOTS:
    void slotToolChanged(KoCanvasController* canvas, int toolId);

Q_SIGNALS:
    void viewChanged();
    void currentToolChanged();

private:
    class Private;
    Private* d;
};

#endif // TOOLMANAGER_H
