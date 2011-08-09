/* This file is part of the KDE project
 *
 * Copyright (c) 2010-2011 Casper Boemann <cbo@boemann.dk>
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

#include <KoDockWidgetTitleBarButton.h>
#include <KoDockWidgetTitleBar.h>

#include <klocale.h>
#include <kdebug.h>
#include <kicon.h>
#include <kconfiggroup.h>
#include <kglobal.h>

#include <QPointer>
#include <QGridLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QLabel>
#include <QSet>
#include <QAction>
#include <QStyleOptionFrame>
#include <QToolButton>
#include <QTabWidget>

class KoToolDocker::Private {
public:
    Private(KoToolDocker *dock)
            : q(dock)
            ,tabbed(false)
            ,hasTitle(false)
    {
        lockIcon = KIcon("object-locked");
        unlockIcon = KIcon("object-unlocked");
        tabIcon = KIcon("tab-new");
        unTabIcon = KIcon("tab-close");
    }

    QList<QWidget *> currentWidgetList;
    QSet<QWidget *> currentAuxWidgets;
    QScrollArea *scrollArea;
    QWidget *hiderWidget; // non current widgets are hidden by being children of this
    QWidget *housekeeperWidget;
    QGridLayout *housekeeperLayout;
    KoToolDocker *q;
    Qt::DockWidgetArea dockingArea;
    bool tabbed;
    bool hasTitle;
    KIcon lockIcon;
    KIcon unlockIcon;
    KIcon tabIcon;
    KIcon unTabIcon;
    QToolButton *lockButton;
    QToolButton *tabButton;

    void resetWidgets()
    {
        currentWidgetList.clear();
        qDeleteAll(currentAuxWidgets);
        currentAuxWidgets.clear();
    }

    void recreateLayout(const QList<QWidget *> &optionWidgetList)
    {
        foreach(QWidget* widget, currentWidgetList) {
            widget->setParent(hiderWidget);
        }
        qDeleteAll(currentAuxWidgets);
        currentAuxWidgets.clear();

        currentWidgetList = optionWidgetList;

        if (tabbed && currentWidgetList.size() > 1) {
            QTabWidget *t;
            housekeeperLayout->addWidget(t = new QTabWidget(), 0, 0);
            currentAuxWidgets.insert(t);
            foreach(QWidget *widget, currentWidgetList) {
                if (widget->objectName().isEmpty()) {
                    Q_ASSERT(!(widget->objectName().isEmpty()));
                    continue; // skip this docker in release build when assert don't crash
                }
                t->addTab(widget, widget->windowTitle());
            }
        } else {
            int cnt = 0;
            QFrame *s;
            QLabel *l;
            switch(dockingArea) {
            case Qt::TopDockWidgetArea:
            case Qt::BottomDockWidgetArea:
                housekeeperLayout->setHorizontalSpacing(2);
                housekeeperLayout->setVerticalSpacing(0);
                foreach(QWidget* widget, currentWidgetList) {
                    if (widget->objectName().isEmpty()) {
                        continue; // skip this docker in release build when assert don't crash
                    }
                    if (!widget->windowTitle().isEmpty()) {
                        housekeeperLayout->addWidget(l = new QLabel(widget->windowTitle()), 0, 2*cnt);
                        currentAuxWidgets.insert(l);
                    }
                    housekeeperLayout->addWidget(widget, 1, 2*cnt);
                    widget->show();
                    if (widget != currentWidgetList.last()) {
                        housekeeperLayout->addWidget(s = new QFrame(), 0, 2*cnt+1, 2, 1);
                        s->setFrameShape(QFrame::VLine);
                        currentAuxWidgets.insert(s);
                    }
                    cnt++;
                }
                break;
            case Qt::LeftDockWidgetArea:
            case Qt::RightDockWidgetArea:
                housekeeperLayout->setHorizontalSpacing(0);
                housekeeperLayout->setVerticalSpacing(2);
                foreach(QWidget *widget, currentWidgetList) {
                    if (widget->objectName().isEmpty()) {
                        Q_ASSERT(!(widget->objectName().isEmpty()));
                        continue; // skip this docker in release build when assert don't crash
                    }
                    if (!widget->windowTitle().isEmpty()) {
                        housekeeperLayout->addWidget(l = new QLabel(widget->windowTitle()), cnt++, 0);
                        currentAuxWidgets.insert(l);
                    }
                    housekeeperLayout->addWidget(widget, cnt++, 0);
                    widget->show();
                    if (widget != currentWidgetList.last()) {
                        housekeeperLayout->addWidget(s = new QFrame(), cnt++, 0);
                        s->setFrameShape(QFrame::HLine);
                        currentAuxWidgets.insert(s);
                    }
                }
                break;
            default:
                break;
            }
        }
        housekeeperLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        housekeeperLayout->invalidate();
    }

    void locationChanged(Qt::DockWidgetArea area)
    {
        dockingArea = area;
        recreateLayout(currentWidgetList);
    }

    void toggleLock()
    {
        if (!hasTitle) {
            q->setTitleBarWidget(new KoDockWidgetTitleBar(q));
            hasTitle = true;
            lockButton->setIcon(unlockIcon);
            tabButton->setVisible(true);
            // parent hack to keep it clickable
            tabButton->setParent(q);
            tabButton->show();
            lockButton->setParent(0);
            lockButton->setParent(q);
            lockButton->show();
        } else {
            q->setTitleBarWidget(new QWidget());
            hasTitle = false;
            lockButton->setIcon(lockIcon);
            tabButton->setVisible(false);
            // parent hack to keep it clickable
            tabButton->setParent(0);
            lockButton->setParent(0);
            lockButton->setParent(q);
            lockButton->show();
        }
        q->resizeEvent(0);
    }
    void toggleTab()
    {
        if (!tabbed) {
            tabbed = true;
            tabButton->setIcon(unTabIcon);
        } else {
            tabbed = false;
            tabButton->setIcon(tabIcon);
        }
        recreateLayout(currentWidgetList);
    }
};

KoToolDocker::KoToolDocker(QWidget *parent)
    : QDockWidget(i18n("Tool Options"), parent),
    d(new Private(this))
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea);

    KConfigGroup cfg = KGlobal::config()->group("DockWidget sharedtooldocker");
    d->tabbed = cfg.readEntry("TabbedMode", false);
    d->hasTitle = cfg.readEntry("Locked", true);

    toggleViewAction()->setVisible(false); //should always be visible, so hide option in menu
    setFeatures(DockWidgetMovable|DockWidgetFloatable);
    if (d->hasTitle) {
        setTitleBarWidget(new KoDockWidgetTitleBar(this));
    } else {
        setTitleBarWidget(new QWidget());
    }
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
    d->scrollArea->setWidgetResizable(true);

    setWidget(d->scrollArea);

    d->lockButton = new QToolButton(this);
    if (d->hasTitle) {
        d->lockButton->setIcon(d->unlockIcon);
    } else {
        d->lockButton->setIcon(d->lockIcon);
    }
    d->lockButton->setToolTip(i18n("Toggles showing a title bar"));
    d->lockButton->setAutoRaise(true);
    connect(d->lockButton, SIGNAL(clicked()), SLOT(toggleLock()));
    d->lockButton->setVisible(true);
    d->lockButton->resize(d->lockButton->sizeHint());

    d->tabButton = new QToolButton(this); // parent hack in toggleLock to keep it clickable
    d->tabButton->setIcon(d->tabIcon);
    d->tabButton->setToolTip(i18n("Toggles organising the options in tabs or not"));
    d->tabButton->setAutoRaise(true);
    connect(d->tabButton, SIGNAL(clicked()), SLOT(toggleTab()));
    d->tabButton->resize(d->tabButton->sizeHint());
    d->tabButton->setVisible(d->hasTitle);
}

KoToolDocker::~KoToolDocker()
{
    KConfigGroup cfg = KGlobal::config()->group("DockWidget sharedtooldocker");
    cfg.writeEntry("TabbedMode", d->tabbed);
    cfg.writeEntry("Locked", d->hasTitle);
    cfg.sync();

    delete d;
}

bool KoToolDocker::hasOptionWidget()
{
    return !d->currentWidgetList.isEmpty();
}

void KoToolDocker::setOptionWidgets(const QList<QWidget *> &optionWidgetList)
{
    d->recreateLayout(optionWidgetList);
}

void KoToolDocker::resizeEvent(QResizeEvent*)
{
    int fw = isFloating() ? style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, this) : 0;
    d->lockButton->move(width() - d->lockButton->width() - d->scrollArea->verticalScrollBar()->sizeHint().width(), fw);
    d->tabButton->move(d->lockButton->x() - d->tabButton->width() - 2, d->lockButton->y());
}

void KoToolDocker::resetWidgets()
{
    d->resetWidgets();
}

#include <KoToolDocker_p.moc>
