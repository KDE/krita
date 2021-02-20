/*
 *  SPDX-FileCopyrightText: 2013 Juan Palacios <jpalaciosdev@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_size_group_p.h"

#include <QEvent>
#include <QTimer>
#include <QWidget>
#include <QLayout>
#include <QGridLayout>
#include <QFormLayout>

KisSizeGroupPrivate::KisSizeGroupPrivate(KisSizeGroup *q_ptr, KisSizeGroup::mode mode, bool ignoreHidden)
    : QObject()
    , q(q_ptr)
    , m_mode(mode)
    , m_ignoreHidden(ignoreHidden)
    , m_updateTimer(new QTimer(q))
    , m_sizeHint(0, 0)
{
    Q_ASSERT(q_ptr);

    m_updateTimer->setSingleShot(true);
    m_updateTimer->setInterval(0);
    QObject::connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(updateSize()));
}

void KisSizeGroupPrivate::addWidget(QWidget *widget)
{
    Q_ASSERT(widget);

    QWidget *parent = widget->parentWidget();
    if (parent) {
        QLayout *layout = parent->layout();
        if (layout) {
            // Search for the widget index and the QLayoutItem inside of the layout
            QLayoutItem *layoutItem =  0;
            int layoutWidgetIndex = 0;
            for(int i = 0; i < layout->count(); ++i) {
                layoutItem = layout->itemAt(layoutWidgetIndex);
                if (layoutItem->widget() == widget) break;
                ++layoutWidgetIndex;
            }

            // We need to replace the layoutItem with an instance of GroupItem
            GroupItem *groupItem = dynamic_cast<GroupItem*>(layoutItem);
            if (groupItem) {
                // This widget is already inside of a group
                // One widget inside multiple groups is not supported
                Q_ASSERT(groupItem->getGroup() == this);
            } else {
                // LayoutItem is not an instance of WidgetItem
                // We need to create a new one

                groupItem = new GroupItem(widget);
                groupItem->setGroup(this);

                // Now we need to replace the layoutItem with the groupItem.
                // This step depends on the actual layout specialization.

                // Widget within a QFormLayout
                QFormLayout* formLayout = qobject_cast<QFormLayout*>(layout);
                if (formLayout) {
                    int row;
                    QFormLayout::ItemRole role;
                    formLayout->getItemPosition(layoutWidgetIndex, &row, &role);
                    formLayout->removeItem(layoutItem);
                    delete layoutItem;
                    formLayout->setItem(row, role, groupItem);
                    m_groupItems.append(groupItem);

                    return;
                }

                // Widget within a QGridLayout
                QGridLayout *gridLayout = qobject_cast<QGridLayout*>(layout);
                if (gridLayout) {
                    int row, column, rowspan, columnspan;
                    gridLayout->getItemPosition(layoutWidgetIndex, &row, &column, &rowspan, &columnspan);
                    gridLayout->removeItem(layoutItem);
                    delete layoutItem;
                    gridLayout->addItem(groupItem, row, column, rowspan, columnspan);
                    m_groupItems.append(groupItem);

                    return;
                }

                // Widget within a QBoxLayout
                QBoxLayout *boxLayout = qobject_cast<QBoxLayout*>(layout);
                if (boxLayout) {
                    boxLayout->removeItem(layoutItem);
                    delete layoutItem;
                    boxLayout->insertItem(layoutWidgetIndex, groupItem);
                    m_groupItems.append(groupItem);

                    return;
                }
            }
        }
    }

}

void KisSizeGroupPrivate::removeWidget(QWidget *widget)
{
    Q_ASSERT(widget);

    QWidget *parent = widget->parentWidget();
    if (parent) {
        QLayout *layout = parent->layout();
        if (layout) {
            // Search the GroupItem of the widget inside of the GroupItem list
            GroupItem *widgetGroupItem = 0;
            Q_FOREACH(GroupItem * groupItem, m_groupItems) {
                if (groupItem->widget() == widget) {
                    widgetGroupItem = groupItem;
                    break;
                }
            }

            if (widgetGroupItem) {
                m_groupItems.removeAll(widgetGroupItem);

                int layoutWidgetIndex = layout->indexOf(widget);

                // Now we need to replace the GroupItem with a QWidgetItem for the widget.
                // This step depends on the actual layout specialization.

                // Widget within a QFormLayout
                QFormLayout* formLayout = qobject_cast<QFormLayout*>(layout);
                if (formLayout) {
                    int row;
                    QFormLayout::ItemRole role;
                    formLayout->getItemPosition(layoutWidgetIndex, &row, &role);
                    formLayout->removeItem(widgetGroupItem);
                    delete widgetGroupItem;
                    formLayout->setWidget(row, role, widget);

                    return;
                }

                // Widget within a QGridLayout
                QGridLayout *gridLayout = qobject_cast<QGridLayout*>(layout);
                if (gridLayout) {
                    int row, column, rowspan, columnspan;
                    gridLayout->getItemPosition(layoutWidgetIndex, &row, &column, &rowspan, &columnspan);
                    gridLayout->removeItem(widgetGroupItem);
                    delete widgetGroupItem;
                    QWidgetItem *widgetItem = new QWidgetItem(widget);
                    gridLayout->addItem(widgetItem, row, column, rowspan, columnspan);

                    return;
                }

                // Widget within a QBoxLayout
                QBoxLayout *boxLayout = qobject_cast<QBoxLayout*>(layout);
                if (boxLayout) {
                    boxLayout->removeItem(widgetGroupItem);
                    delete widgetGroupItem;
                    QWidgetItem *widgetItem = new QWidgetItem(widget);
                    boxLayout->insertItem(layoutWidgetIndex, widgetItem);

                    return;
                }
            }
        }
    }
}

void KisSizeGroupPrivate::scheduleSizeUpdate()
{
    m_updateTimer->start();
}

void KisSizeGroupPrivate::updateSize()
{
    if (m_mode == KisSizeGroup::KIS_SIZE_GROUP_NONE) {
        // restore original widget size in each GroupItem
        Q_FOREACH(GroupItem *groupItem, m_groupItems) {
            groupItem->setSize(groupItem->widget()->sizeHint());
            groupItem->widget()->updateGeometry();
        }
    } else {
        // compute widgets size
        int width = 0;
        int height = 0;
        Q_FOREACH(GroupItem *groupItem, m_groupItems) {
            if (m_ignoreHidden && groupItem->hidden())
                continue;

            const QWidget *widget = groupItem->widget();
            width = qMax(widget->sizeHint().width(), width);
            height = qMax(widget->sizeHint().height(), height);
        }

        m_sizeHint.setWidth(width);
        m_sizeHint.setHeight(height);

        // update groupItem size
        Q_FOREACH(GroupItem *groupItem, m_groupItems) {
            if (m_ignoreHidden && groupItem->hidden())
                continue;

            switch(m_mode) {
            case KisSizeGroup::KIS_SIZE_GROUP_HORIZONTAL:
                groupItem->setWidth(width);
                break;

            case KisSizeGroup::KIS_SIZE_GROUP_VERTICAL:
                groupItem->setHeight(height);
                break;

            case KisSizeGroup::KIS_SIZE_GROUP_BOTH:
                groupItem->setWidth(width);
                groupItem->setHeight(height);
                break;

            default:
                break;
            }

            // update layout to the new groupItem size
            groupItem->widget()->updateGeometry();

        }
    }
}

GroupItem::GroupItem(QWidget* widget)
    : QObject()
    , QWidgetItem(widget)
{
    Q_ASSERT(widget);
    m_size = widget->sizeHint();
    m_hidden = !widget->isVisible();
    widget->installEventFilter(this);
}

QSize GroupItem::sizeHint() const
{
    return m_size;
}

QSize GroupItem::minimumSize() const
{
    QSize size = QWidgetItem::minimumSize();
    if (m_group->m_mode != KisSizeGroup::KIS_SIZE_GROUP_NONE) {
        size = m_group->getSizeHint();
    }
    return size;
}


bool GroupItem::eventFilter(QObject*, QEvent *event)
{
    switch (event->type()) {
    case QEvent::Hide:
        if (!event->spontaneous()) {
            m_hidden = true;
            m_group->scheduleSizeUpdate();
        }
        break;

    case QEvent::Show:
        if (!event->spontaneous()) {
            m_hidden = false;
            m_group->scheduleSizeUpdate();
        }
        break;

    case QEvent::Resize:
        m_group->scheduleSizeUpdate();
        break;

    default:
        break;
    }

    return false;
}
