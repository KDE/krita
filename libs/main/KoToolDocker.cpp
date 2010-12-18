/* This file is part of the KDE project
 *
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

class KoToolDocker::Private {
public:
    Private(KoToolDocker *dock) : q(dock) {}

    ~Private()
    {
/*        QMap<QString, QWidget*>::ConstIterator iter = currentWidgetMap.constBegin();
        for (;iter != currentWidgetMap.constEnd(); ++iter) {
            iter.value()->setParent(0);
        }
*/    }

    QMap<QString, QWidget *> currentWidgetMap;
    QScrollArea *scrollArea;
    QWidget *housekeeperWidget;
    QGridLayout *housekeeperLayout;
    KoToolDocker *q;

    void locationChanged(Qt::DockWidgetArea area)
    {
        switch(area) {
        case Qt::TopDockWidgetArea:
        case Qt::BottomDockWidgetArea:
            break;
        case Qt::LeftDockWidgetArea:
        case Qt::RightDockWidgetArea:
            break;
        default:
            break;
        }
        housekeeperLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        housekeeperLayout->invalidate();
    }
};

KoToolDocker::KoToolDocker(QWidget *parent)
    : QDockWidget("Tool Options", parent),
    d(new Private(this))
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea);

    setFeatures(NoDockWidgetFeatures);

    setTitleBarWidget(new QWidget());
    connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea )), this, SLOT(locationChanged(Qt::DockWidgetArea)));

    d->housekeeperWidget = new QWidget();
    d->housekeeperLayout = new QGridLayout();
    d->housekeeperWidget->setLayout(d->housekeeperLayout);
    d->housekeeperLayout->setHorizontalSpacing(0);
    d->housekeeperLayout->setVerticalSpacing(2);
    d->housekeeperLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

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
    QMap<QString, QWidget*>::ConstIterator iter = d->currentWidgetMap.constBegin();
    for (;iter != d->currentWidgetMap.constEnd(); ++iter) {
        iter.value()->setVisible(false);
        iter.value()->setParent(0);
    }

    // Now add option widgets to docker
    iter = optionWidgetMap.constBegin();
    int cnt=0;
    QFrame *s;
    for (;iter != optionWidgetMap.constEnd(); ++iter) {
        if (iter.value()->objectName().isEmpty()) {
            kError(30004) << "tooldocker widget have no name " << iter.key();
            Q_ASSERT(!(iter.value()->objectName().isEmpty()));
            continue; // skip this docker in release build when assert don't crash
        }
        d->housekeeperLayout->addWidget(new QLabel(iter.key()), 3*cnt, 0);
        d->housekeeperLayout->addWidget(iter.value(), 3*cnt+1, 0);
        d->housekeeperLayout->addWidget(s = new QFrame(), 3*cnt+2, 0);
        s->setFrameShape(QFrame::HLine);
        ++cnt;
    }

    d->currentWidgetMap = optionWidgetMap;
    d->housekeeperLayout->invalidate();
}

#include <KoToolDocker_p.moc>
