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
#include <QRect>
#include <QAbstractButton>


class SectionLayout : public QLayout
{
public:
    explicit SectionLayout(QWidget *parent)
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
    enum SeparatorFlag {
        SeparatorTop = 0x0001,/* SeparatorBottom = 0x0002, SeparatorRight = 0x0004,*/ SeparatorLeft = 0x0008
    };
    Q_DECLARE_FLAGS(Separators, SeparatorFlag)
    explicit Section(QWidget *parent = 0)
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

    void setSeparator(Separators separators)
    {
        m_separators = separators;
    }

    Separators separators() const
    {
        return m_separators;
    }

    void setOrientation (Qt::Orientation orientation)
    {
        m_layout->setOrientation(orientation);
    }

private:
    SectionLayout *m_layout;
    QString m_name;
    Separators m_separators;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Section::Separators)

class KoToolBoxLayout : public QLayout
{
public:
    explicit KoToolBoxLayout(QWidget *parent)
        : QLayout(parent), m_orientation(Qt::Vertical), m_currentHeight(0)
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
            s.setWidth(m_currentHeight);
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
        // nothing to do?
        if (m_sections.isEmpty())
        {
            m_currentHeight = 0;
            return;
        }

        // the names of the variables assume a vertical orientation,
        // but all calculations are done based on the real orientation

        const QSize iconSize = static_cast<Section*> (m_sections.first()->widget())->iconSize();

        const int maxWidth = (m_orientation == Qt::Vertical) ? rect.width() : rect.height();
        // using min 1 as width to e.g. protect against div by 0 below
        const int iconWidth = qMax(1, (m_orientation == Qt::Vertical) ? iconSize.width() : iconSize.height());
        const int iconHeight = qMax(1, (m_orientation == Qt::Vertical) ? iconSize.height() : iconSize.width());

        const int maxColumns = qMax(1, (maxWidth / iconWidth));

        int x = 0;
        int y = 0;
        bool firstSection = true;
        foreach (QWidgetItem *wi, m_sections) {
            Section *section = static_cast<Section*> (wi->widget());
            const int buttonCount = section->visibleButtonCount();
            if (buttonCount == 0) {
                // move out of view, not perfect TODO: better solution
                section->setGeometry(1000, 1000, 0, 0);
                continue;
            }

            // rows needed for the buttons (calculation gets the ceiling value of the plain div)
            const int neededRowCount = ((buttonCount-1) / maxColumns) + 1;
            const int availableButtonCount = (maxWidth - x + 1) / iconWidth;

            if (firstSection) {
                firstSection = false;
            } else if (buttonCount > availableButtonCount) {
                // start on a new row, set separator
                x = 0;
                y += iconHeight + spacing();
                const Section::Separators separator =
                    (m_orientation == Qt::Vertical) ? Section::SeparatorTop : Section::SeparatorLeft;
                section->setSeparator( separator );
            } else {
                // append to last row, set separators (on first row only to the left side)
                const bool isFirstRow = (y == 0);
                const Section::Separators separators =
                    isFirstRow ?
                        ((m_orientation == Qt::Vertical) ? Section::SeparatorLeft : Section::SeparatorTop) :
                        (Section::SeparatorTop | Section::SeparatorLeft);
                section->setSeparator( separators );
            }

            const int usedColumns = qMin(buttonCount, maxColumns);
            if (m_orientation == Qt::Vertical) {
                section->setGeometry(x, y,
                                     usedColumns * iconWidth, neededRowCount * iconHeight);
            } else {
                section->setGeometry(y, x,
                                     neededRowCount * iconHeight, usedColumns * iconWidth);
            }

            // advance by the icons in the last row
            const int lastRowColumnCount = buttonCount - ((neededRowCount-1) * maxColumns);
            x += (lastRowColumnCount * iconWidth) + spacing();
            // advance by all but the last used row
            y += (neededRowCount - 1) * iconHeight;
        }

        // cache total height (or width), adding the iconHeight for the current row
        m_currentHeight = y + iconHeight;
    }

    void setOrientation (Qt::Orientation orientation)
    {
        m_orientation = orientation;
        invalidate();
    }

private:
    QList <QWidgetItem*> m_sections;
    Qt::Orientation m_orientation;
    mutable int m_currentHeight;
};

#endif
