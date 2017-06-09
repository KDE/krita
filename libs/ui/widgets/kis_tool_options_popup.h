/* This file is part of the KDE project
 * Copyright (C) 2015 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_TOOL_OPTIONS_POPUP_H
#define KIS_TOOL_OPTIONS_POPUP_H

#include <QWidget>
#include <QPointer>

class KisToolOptionsPopup : public QWidget
{
    Q_OBJECT
public:
    explicit KisToolOptionsPopup(QWidget *parent = 0);
    ~KisToolOptionsPopup() override;

    bool detached() const;

    void newOptionWidgets(const QList<QPointer<QWidget> > &optionWidgetList);

Q_SIGNALS:

public Q_SLOTS:

    void switchDetached(bool show = true);

protected:
    void contextMenuEvent(QContextMenuEvent *) override;
    void hideEvent(QHideEvent *) override;
    void showEvent(QShowEvent *) override;

private:

    struct Private;
    Private *const d;
};

#endif // KIS_TOOL_OPTIONS_POPUP_H
