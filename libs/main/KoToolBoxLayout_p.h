/*
 * Copyright (c) 2005-2009 Thomas Zander <zander@kde.org>
 * Copyright (c) 2009 Peter Simonsson <peter.simonsson@gmail.com>
 * Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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
#ifndef _KO_TOOLBOX_LAYOUT_H_
#define _KO_TOOLBOX_LAYOUT_H_

#include <KDebug>
#include <QLayout>
#include <QMap>
#include <QButtonGroup>
#include <QToolButton>
#include <QHash>
#include <QPainter>
#include <QRect>
#include <QTimer>

#include "math.h"

class SectionLayout : public QLayout
{
public:
    SectionLayout(QWidget *parent)
        : QLayout(parent), m_orientation(Qt::Vertical)
    {
    }

    ~SectionLayout()
    {
        qDeleteAll( m_items );
        m_items.clear();
    }

    void addButton(QAbstractButton *button, int priority)
    {
        addChildWidget(button);
        m_priorities.insert(button, priority);
        int index = 1;
        foreach(QWidgetItem *item, m_items) {
            if (m_priorities.value(static_cast<QAbstractButton*>(item->widget())) > priority)
                break;
            index++;
        }
        m_items.insert(index-1, new QWidgetItem(button));
    }

    QSize sizeHint() const
    {
        Q_ASSERT(0);
        return QSize();
    }

    void addItem(QLayoutItem*) { Q_ASSERT(0); }
    QLayoutItem* itemAt(int i) const
    {
        if (m_items.count() <= i)
            return 0;
        return m_items.at(i);
    }

    QLayoutItem* takeAt(int i) { return m_items.takeAt(i); }
    int count() const { return m_items.count(); }

    void setGeometry (const QRect &rect)
    {
        int x = 0;
        int y = 0;
        const QSize &size = buttonSize();
        if (m_orientation == Qt::Vertical) {
            foreach (QWidgetItem* w, m_items) {
                if (w->isEmpty())
                    continue;
                w->widget()->setGeometry(QRect(x, y, size.width(), size.height()));
                x += size.width();
                if (x + size.width() > rect.width()) {
                    x = 0;
                    y += size.height();
                }
            }
        } else {
            foreach (QWidgetItem* w, m_items) {
                if (w->isEmpty())
                    continue;
                w->widget()->setGeometry(QRect(x, y, size.width(), size.height()));
                y += size.height();
                if (y + size.height() > rect.height()) {
                    x += size.width();
                    y = 0;
                }
            }
        }
    }

    const QSize &buttonSize() const
    {
        if (!m_items.isEmpty() && ! m_buttonSize.isValid())
            const_cast<SectionLayout*> (this)->m_buttonSize = m_items[0]->widget()->sizeHint();
        return m_buttonSize;
    }

    void setOrientation (Qt::Orientation orientation)
    {
        m_orientation = orientation;
    }

private:
    QSize m_buttonSize;
    QMap<QAbstractButton*, int> m_priorities;
    QList<QWidgetItem*> m_items;
    Qt::Orientation m_orientation;
};

class Section : public QWidget
{
public:
    enum SeperatorFlag {
        SeperatorTop = 0x0001,/* SeperatorBottom = 0x0002, SeperatorRight = 0x0004,*/ SeperatorLeft = 0x0008
    };
    Q_DECLARE_FLAGS(Seperators, SeperatorFlag)
    Section(QWidget *parent = 0)
        : QWidget(parent),
        m_layout(new SectionLayout(this))
    {
        setLayout(m_layout);
    }

    void addButton(QAbstractButton *button, int priority)
    {
        m_layout->addButton(button, priority);
    }

    void setName(const QString &name)
    {
        m_name = name;
    }

    QString name() const
    {
        return m_name;
    }

    QSize iconSize() const
    {
        return m_layout->buttonSize();
    }

    int visibleButtonCount() const
    {
        int count = 0;
        for(int i = m_layout->count()-1; i >= 0; --i) {
            if (! static_cast<QWidgetItem*> (m_layout->itemAt(i))->isEmpty())
                ++count;
        }
        return count;
    }

    void setSeperator(Seperators seperators)
    {
        m_seperators = seperators;
    }

    Seperators seperators() const
    {
        return m_seperators;
    }

    void setOrientation (Qt::Orientation orientation)
    {
        m_layout->setOrientation(orientation);
    }

private:
    SectionLayout *m_layout;
    QString m_name;
    Seperators m_seperators;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Section::Seperators)

class KoToolBoxLayout : public QLayout
{
public:
    KoToolBoxLayout(QWidget *parent)
        : QLayout(parent), m_orientation(Qt::Vertical), m_currentHeight(0), m_currentWidth(0)
    {
        setSpacing(6);
    }

    ~KoToolBoxLayout()
    {
        qDeleteAll( m_sections );
        m_sections.clear();
    }

    QSize sizeHint() const
    {
        if (m_sections.isEmpty())
            return QSize();
        QSize oneIcon = static_cast<Section*> (m_sections[0]->widget())->iconSize();
        return oneIcon;
    }
    QSize minimumSize() const
    {
        QSize s = sizeHint();
        if (m_orientation == Qt::Vertical) {
            s.setHeight(m_currentHeight);
        } else {
            s.setWidth(m_currentWidth);
        }
        return s;
    }

    void addSection(Section *section)
    {
        addChildWidget(section);

        QList<QWidgetItem*>::iterator iterator = m_sections.begin();
        int defaults = 2; // skip the first two as they are the 'main' and 'dynamic' sections.
        while (iterator != m_sections.end()) {
            if (--defaults < 0 && static_cast<Section*> ((*iterator)->widget())->name() > section->name())
                break;
            ++iterator;
        }
        m_sections.insert(iterator, new QWidgetItem(section));
    }

    void addItem(QLayoutItem*)
    {
        Q_ASSERT(0); // don't let anything else be added. (code depends on this!)
    }

    QLayoutItem* itemAt(int i) const
    {
        if (m_sections.count() >= i)
            return 0;
        return m_sections.at(i);
    }
    QLayoutItem* takeAt(int i) { return m_sections.takeAt(i); }
    int count() const { return m_sections.count(); }

    void setGeometry (const QRect &rect)
    {
        if (m_orientation == Qt::Vertical) {
            tryPlaceItems(rect.width(), true);
        } else {
            tryPlaceItems(rect.height(), true);
        }
    }

    /// returns height
    int tryPlaceItems(int width, bool actuallyPlace) const
    {
        if (m_sections.isEmpty())
            return 0;
        QSize iconSize = static_cast<Section*> (m_sections[0]->widget())->iconSize();
        const int maxColumns = qMax(1, width / iconSize.width());

        int x = 0;
        int y = 0;
        int unusedButtons = 0;
        bool firstSection = true;
        foreach (QWidgetItem *wi, m_sections) {
            Section *section = static_cast<Section*> (wi->widget());
            const int buttonCount = section->visibleButtonCount();
            if (buttonCount == 0) {
                if (actuallyPlace)
                    section->setGeometry(1000, 1000, 0, 0);
                continue;
            }
            // kDebug() << " + section" << buttonCount;
            int rows = (int) ceilf(buttonCount / (float) maxColumns);

            int length = 0;
            if (firstSection) {
                firstSection = false;
                unusedButtons = rows * maxColumns;
            } else if (buttonCount > unusedButtons) {
                if (m_orientation == Qt::Vertical) {
                    y += iconSize.height() + spacing();
                    section->setSeperator(Section::SeperatorTop);
                } else {
                    x += iconSize.height() + spacing();
                    section->setSeperator(Section::SeperatorLeft);
                }
                unusedButtons = rows * maxColumns;
            } else {
                if (m_orientation == Qt::Vertical) {
                    length = (maxColumns - unusedButtons) * iconSize.width();
                    x = length + spacing();
                    section->setSeperator(Section::SeperatorTop | Section::SeperatorLeft);
                } else {
                    length = (maxColumns - unusedButtons) * iconSize.height();
                    y = length + spacing();
                    section->setSeperator(Section::SeperatorTop | Section::SeperatorLeft);
                }
            }

            if (actuallyPlace) {
                if (m_orientation == Qt::Vertical) {
                    section->setGeometry(QRect(x, y, maxColumns * iconSize.width() - length,
                                               rows * iconSize.height()));
                } else {
                    section->setGeometry(QRect(x, y, rows * iconSize.width(),
                                               maxColumns * iconSize.height() - length));
                }
            }

            unusedButtons -= buttonCount;

            if (m_orientation == Qt::Vertical) {
                x = 0;
                y += (rows - 1) * iconSize.height();
            } else {
                y = 0;
                x += (rows - 1) * iconSize.height();
            }
        }
        m_currentWidth = x;
        m_currentHeight = y;
        if (m_orientation == Qt::Vertical) {
            m_currentHeight += iconSize.height();
        } else {
            m_currentWidth += iconSize.height();
        }
        return m_currentHeight;
    }

    void setOrientation (Qt::Orientation orientation)
    {
        m_orientation = orientation;
        invalidate();
    }

private:
    QList <QWidgetItem*> m_sections;
    Qt::Orientation m_orientation;
    mutable int m_currentHeight, m_currentWidth;
};

#endif
