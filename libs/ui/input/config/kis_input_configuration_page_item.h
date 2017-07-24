/*
 * This file is part of the KDE project
 * Copyright (C) 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISINPUTCONFIGURATIONPAGEITEM_H
#define KISINPUTCONFIGURATIONPAGEITEM_H

#include <QWidget>

namespace Ui
{
class KisInputConfigurationPageItem;
}

class KisActionShortcutsModel;
class KisAbstractInputAction;
/**
 * \brief A collapsible widget displaying an action, its description and associated shortcuts.
 *
 * This is used in KisInputConfigurationPage to display a list of actions and the associated
 * shortcuts, depending on the current profile.
 */
class KisInputConfigurationPageItem : public QWidget
{
    Q_OBJECT
public:
    KisInputConfigurationPageItem(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~KisInputConfigurationPageItem() override;

    void setAction(KisAbstractInputAction *action);

public Q_SLOTS:
    void setExpanded(bool expand);

private Q_SLOTS:
    void deleteShortcut();

private:
    Ui::KisInputConfigurationPageItem *ui;
    KisAbstractInputAction *m_action;
    KisActionShortcutsModel *m_shortcutsModel;
};

#endif // KISINPUTCONFIGURATIONPAGEITEM_H
