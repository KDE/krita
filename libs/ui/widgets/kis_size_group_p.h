/*
 *  Copyright (C) 2013 Juan Palacios <jpalaciosdev@gmail.com>
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

#ifndef KISSIZEGROUPPRIVATE_H
#define KISSIZEGROUPPRIVATE_H

#include <QObject>
#include <QWidgetItem>
#include <QList>
#include <QSize>

#include "kis_size_group.h"

class QWidget;
class QEvent;
class QTimer;

class GroupItem;
class KisSizeGroupPrivate : public QObject
{
    Q_OBJECT

public:
    KisSizeGroupPrivate(KisSizeGroup *q_ptr, KisSizeGroup::mode mode, bool ignoreHidden);

    void addWidget(QWidget *widget);
    void removeWidget(QWidget *widget);

    /// Schedules an update of all widgets size
    void scheduleSizeUpdate();

    /// Returns the size hint for the widgets contained in the size group.
    const QSize getSizeHint() const { return m_sizeHint; }


private Q_SLOTS:
    void updateSize();

public:
    KisSizeGroup* q;
    KisSizeGroup::mode m_mode;
    bool m_ignoreHidden;

private:
    QTimer* m_updateTimer; // used to filter multiple calls to scheduleSizeUpdate() into one single updateSize()
    QList<GroupItem*> m_groupItems;
    QSize m_sizeHint;
};


class GroupItem : public QObject, public QWidgetItem
{
    Q_OBJECT

public:
    explicit GroupItem(QWidget* widget);
    ~GroupItem() override {}

    void setSize(const QSize &size) { m_size = size; }

    int getWidth() const { return m_size.width(); }
    void setWidth(int width) { m_size.setWidth(width); }

    int getHeight() const { return m_size.height(); }
    void setHeight(int height) { m_size.setHeight(height); }

    bool hidden() const { return m_hidden; }

    KisSizeGroupPrivate* getGroup() { return m_group; }
    void setGroup(KisSizeGroupPrivate* group) { m_group = group; }

    QSize sizeHint() const override;
    QSize minimumSize() const override;

    bool eventFilter(QObject*, QEvent *event) override;

private:
    bool m_hidden {false};
    QSize m_size;
    KisSizeGroupPrivate* m_group {0};
};

#endif // KISSIZEGROUPPRIVATE_H
