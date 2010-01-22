/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include <KoCanvasObserverBase.h>

#include <QDockWidget>
#include <QHash>

#include <ui_Panel.h>

class KAction;
class KoTextEditor;

class Panel : public QDockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    Panel(QWidget *parent = 0);
    ~Panel();

    virtual void setCanvas (KoCanvasBase *canvas);

private slots:
    void toolChangeDetected(const QString &toolId);
    void resourceChanged (int key, const QVariant &value);

    void style1ButtonClicked();
    void style2ButtonClicked();
    void style3ButtonClicked();

private:
    void setInitialButtonIcon(QToolButton *button, const QString &name) const;
    void applyAction(KAction *action, QToolButton *button, const QString &iconName, bool partOfGroup);

    KoCanvasBase *m_canvas;
    QObject *m_parent;
    KAction *m_style1, *m_style2, *m_style3;
    KoTextEditor *m_handler;

    Ui::Panel widget;
};

#endif
