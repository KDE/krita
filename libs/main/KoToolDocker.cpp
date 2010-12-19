/* This file is part of the KDE project
 *
 * Copyright (c) 2010 Casper Boemann <cbo@boemann.dk>
 * Copyright (c) 2005-2006 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (c) 2006 Thomas Zander <zander@kde.org>
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
#include "KoToolDocker_p.h"
#include <QPointer>

#include <klocale.h>
#include <kdebug.h>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QSet>

class KoToolDocker::Private {
public:
    Private(KoToolDocker *dock) : q(dock), tabbed(false) {}

    QMap<QString, QWidget *> currentWidgetMap;
    QSet<QWidget *> currentAuxWidgets;
    QScrollArea *scrollArea;
    QWidget *hiderWidget; // non current widgets are hidden by being children of this
    QWidget *housekeeperWidget;
    QGridLayout *housekeeperLayout;
    KoToolDocker *q;
    Qt::DockWidgetArea dockingArea;
    bool tabbed;

    void recreateLayout(const QMap<QString, QWidget *> &optionWidgetMap)
    {
        QMap<QString, QWidget*>::ConstIterator iter = currentWidgetMap.constBegin();

        for (;iter != currentWidgetMap.constEnd(); ++iter) {
            iter.value()->setParent(hiderWidget);
        }
        qDeleteAll(currentAuxWidgets);
        currentAuxWidgets.clear();

        currentWidgetMap = optionWidgetMap;

        // Now add option widgets to docker based on layout area and if it should be tabbed
        // small hack with 128 to cover all in one switch
        switch(dockingArea + (tabbed ? 128 : 0)) {
        case Qt::TopDockWidgetArea:
        case Qt::BottomDockWidgetArea:
        case Qt::TopDockWidgetArea+128: // we don't do tabbed at top
        case Qt::BottomDockWidgetArea+128: // we don't do tabbed at top
            housekeeperLayout->setHorizontalSpacing(2);
            housekeeperLayout->setVerticalSpacing(0);
            iter = currentWidgetMap.constBegin();
            for (int cnt = 0; iter != currentWidgetMap.constEnd(); ++cnt) {
                QFrame *s;
                QLabel *l;
                if (iter.value()->objectName().isEmpty()) {
                    Q_ASSERT(!(iter.value()->objectName().isEmpty()));
                    continue; // skip this docker in release build when assert don't crash
                }
                housekeeperLayout->addWidget(l = new QLabel(iter.key()), 0, 2*cnt);
                currentAuxWidgets.insert(l);
                housekeeperLayout->addWidget(iter.value(), 1, 2*cnt);
                ++iter;
                if (iter != currentWidgetMap.constEnd()) {
                    housekeeperLayout->addWidget(s = new QFrame(), 0, 2*cnt+1, 2, 1);
                    s->setFrameShape(QFrame::VLine);
                    currentAuxWidgets.insert(s);
                }
            }
            break;
        case Qt::LeftDockWidgetArea:
        case Qt::RightDockWidgetArea:
            housekeeperLayout->setHorizontalSpacing(0);
            housekeeperLayout->setVerticalSpacing(2);
            iter = currentWidgetMap.constBegin();
            for (int cnt = 0; iter != currentWidgetMap.constEnd(); ++cnt) {
                QFrame *s;
                QLabel *l;
                if (iter.value()->objectName().isEmpty()) {
                    Q_ASSERT(!(iter.value()->objectName().isEmpty()));
                    continue; // skip this docker in release build when assert don't crash
                }
                housekeeperLayout->addWidget(l = new QLabel(iter.key()), 3*cnt, 0);
                currentAuxWidgets.insert(l);
                housekeeperLayout->addWidget(iter.value(), 3*cnt+1, 0);
                ++iter;
                if (iter != currentWidgetMap.constEnd()) {
                    housekeeperLayout->addWidget(s = new QFrame(), 3*cnt+2, 0);
                    s->setFrameShape(QFrame::HLine);
                    currentAuxWidgets.insert(s);
                }
            }
            break;
        default:
            break;
        }
        housekeeperLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        housekeeperLayout->invalidate();
    }

    void locationChanged(Qt::DockWidgetArea area)
    {
        dockingArea = area;
        recreateLayout(currentWidgetMap);
    }
};

KoToolDocker::KoToolDocker(QWidget *parent)
    : QDockWidget("Tool Options", parent),
    d(new Private(this))
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea);

    //setFeatures(NoDockWidgetFeatures);
    setFeatures(AllDockWidgetFeatures);

    setTitleBarWidget(new QWidget());
    connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea )), this, SLOT(locationChanged(Qt::DockWidgetArea)));

    d->housekeeperWidget = new QWidget();
    d->housekeeperLayout = new QGridLayout();
    d->housekeeperWidget->setLayout(d->housekeeperLayout);
    d->housekeeperLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    d->hiderWidget = new QWidget(d->housekeeperWidget);
    d->hiderWidget->setVisible(false);

    d->scrollArea = new QScrollArea();
    d->scrollArea->setWidget(d->housekeeperWidget);
    d->scrollArea->setFrameShape(QFrame::NoFrame);

    setWidget(d->scrollArea);
}

KoToolDocker::~KoToolDocker()
{
    delete d;
}

bool KoToolDocker::hasOptionWidget()
{
    return !d->currentWidgetMap.isEmpty();
}

void KoToolDocker::setOptionWidgets(const QMap<QString, QWidget *> &optionWidgetMap)
{
    d->recreateLayout(optionWidgetMap);
}

#include <KoToolDocker_p.moc>
