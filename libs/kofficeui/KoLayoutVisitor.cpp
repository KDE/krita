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

#include "KoLayoutVisitor.h"

#include <QLabel>
#include <QLinkedList>
#include <QGridLayout>

#include <KDebug>

struct Item {
    QLabel *label;
    int x;
};

class KoLayoutVisitor::Private {
public:

    void addItem(const Item &item) {
        QLinkedList<Item>::iterator iter = items.begin();
        while(iter->x < item.x && iter != items.end())
            iter++;
        //iter--;
        items.insert(iter, item);
    }

    QLinkedList<Item> items;
};

KoLayoutVisitor::KoLayoutVisitor()
    :d( new Private())
{
}

KoLayoutVisitor::~KoLayoutVisitor() {
    delete d;
}

void KoLayoutVisitor::visit(QWidget *widget) {
    //if(!widget->isVisible()) return;
    QLabel *label = qobject_cast<QLabel*> (widget);
    if(label && !label->text().isEmpty()) {
        Item item;
        item.label = label;
        item.x = label->mapToGlobal(QPoint(0,0)).x();
// QPoint pos = label->mapToGlobal(QPoint(0,0));
// kDebug() << "Found label: " << label->text() << "  at " << pos << endl;
        d->addItem(item);
    }

    QList<QWidget*> childList = widget->findChildren<QWidget*>();
    foreach ( QWidget *w , childList ) {
        // Ignore unless we have the direct parent
        if(qobject_cast<QWidget *>(w->parent()) != widget) continue;
        visit(w);
    }
}

void KoLayoutVisitor::relayout() {
    class Layouter {
    public:
        Layouter() : x(-20), right(0) {}
        void add(const Item &item) {
            //kDebug() << "relayout::add " << item.label->text() << endl;

            QGridLayout *layout = 0;
            foreach ( QGridLayout *l, item.label->parentWidget()->findChildren<QGridLayout*>()) {
                if(l->indexOf(item.label) != -1) {
                    layout = l;
                    break;
                }
            }

            if(layout == 0)
                return;
            if(qAbs(item.x - x) > 16) { // next column.
                alterColumnWidth();
                x = item.x + 8;
                right = 0;
            }

            right = qMax(right, item.x + item.label->width());
            if(! column.contains(layout))
                column.insert(layout, item.label);

            layout->setAlignment(item.label, Qt::AlignRight | Qt::AlignVCenter);
            item.label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        }
        void finish() {
            alterColumnWidth();
        }
    private:
        void alterColumnWidth() {
            if(column.size() <= 1) { // no need to 'sync' 1 layout, or less.
                column.clear();
                return;
            }

            foreach(QGridLayout* layout, column.keys()) {
                QLabel *label = column[layout];
                //kDebug() << "altering 1 layout (" << label->text() << ")\n";
                int index = layout->indexOf(label);
                int row, column, rowSpan, columnSpan;
                layout->getItemPosition(index, &row, &column, &rowSpan, &columnSpan);
                if(columnSpan > 1)
                    kWarning() << "Attention, colSpan for " << label->text() << " is " << columnSpan << endl;
                layout->setColumnMinimumWidth(column, right -label->mapToGlobal(QPoint(0,0)).x());
            }
            column.clear();
        }

        int x, right;
        QHash<QGridLayout*, QLabel*> column;
    };

    Layouter layouter;
    foreach(Item item, d->items)
        layouter.add(item);
    layouter.finish();
}
