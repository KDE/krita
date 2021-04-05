/*
    SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    void slotToolChanged(KoCanvasController* canvas);

Q_SIGNALS:
    void viewChanged();
    void currentToolChanged();

private:
    class Private;
    Private* d;
};

#endif // TOOLMANAGER_H
