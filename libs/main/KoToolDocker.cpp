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
#include <QWidget>

class KoToolDocker::Private {
public:
    Private(KoToolDocker *dock) : q(dock) {}

    ~Private()
    {
        QWidget *widget = currentWidget.data();
        if (widget)
            widget->setParent(0);
    }

    QWeakPointer<QWidget> currentWidget;
    QWidget *housekeeperWidget;
    QGridLayout *housekeeperLayout;
    QSpacerItem *bottomRightSpacer;
    KoToolDocker *q;

    void locationChanged(Qt::DockWidgetArea area)
    {
        switch(area) {
        case Qt::TopDockWidgetArea:
        case Qt::BottomDockWidgetArea:
            // make the spacer take vertical space
            bottomRightSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
            break;
        case Qt::LeftDockWidgetArea:
        case Qt::RightDockWidgetArea:
            // make the spacer take no space
            bottomRightSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
            break;
        default:
            break;
        }
        housekeeperLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        housekeeperLayout->invalidate();
    }
};

KoToolDocker::KoToolDocker(QWidget *parent)
    : QDockWidget("Tool Options initial name - never seen", parent),
    d(new Private(this))
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea);
    connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea )), this, SLOT(locationChanged(Qt::DockWidgetArea)));

    d->housekeeperWidget = new QWidget();
    d->housekeeperLayout = new QGridLayout();
    d->housekeeperWidget->setLayout(d->housekeeperLayout);
    d->housekeeperLayout->setHorizontalSpacing(0);
    d->housekeeperLayout->setVerticalSpacing(0);
    d->housekeeperLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    d->bottomRightSpacer = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    d->housekeeperLayout->addItem(d->bottomRightSpacer, 1, 1);

    setWidget(d->housekeeperWidget);
}

KoToolDocker::~KoToolDocker()
{
    delete d;
}

bool KoToolDocker::hasOptionWidget()
{
    return !d->currentWidget.isNull();
}

void KoToolDocker::newOptionWidget(QWidget *optionWidget)
{
    if (d->currentWidget.data() == optionWidget)
        return;

    QWidget *currentWidget = d->currentWidget.data();
    if (currentWidget) {
        currentWidget->setVisible(false);
    }
    d->currentWidget = optionWidget;
    optionWidget->setVisible(true);
    d->housekeeperLayout->addWidget(optionWidget, 0, 0);
    d->housekeeperLayout->invalidate();
}

#include <KoToolDocker_p.moc>
