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

    QMap<QString, QWidget *> currentWidgetMap;
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

    void recreateLayout(const QMap<QString, QWidget *> &optionWidgetMap)
    {
        QMap<QString, QWidget*>::ConstIterator iter = currentWidgetMap.constBegin();

        for (;iter != currentWidgetMap.constEnd(); ++iter) {
            iter.value()->setParent(hiderWidget);
        }
        qDeleteAll(currentAuxWidgets);
        currentAuxWidgets.clear();

        currentWidgetMap = optionWidgetMap;

        if (tabbed && currentWidgetMap.size() > 1) {
            QTabWidget *t;
            housekeeperLayout->addWidget(t = new QTabWidget(), 0, 0);
            currentAuxWidgets.insert(t);
            iter = currentWidgetMap.constBegin();
            for (int cnt = 0; iter != currentWidgetMap.constEnd(); ++cnt) {
                if (iter.value()->objectName().isEmpty()) {
                    Q_ASSERT(!(iter.value()->objectName().isEmpty()));
                    continue; // skip this docker in release build when assert don't crash
                }
                t->addTab(iter.value(), iter.key());
                iter.value()->show();
                ++iter;
            }
        } else {
            switch(dockingArea) {
            case Qt::TopDockWidgetArea:
            case Qt::BottomDockWidgetArea:
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
                    iter.value()->show();
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
                    iter.value()->show();
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
        }
        housekeeperLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        housekeeperLayout->invalidate();
    }

    void locationChanged(Qt::DockWidgetArea area)
    {
        dockingArea = area;
        recreateLayout(currentWidgetMap);
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
        lockButton->move(q->width() - lockButton->width() - scrollArea->verticalScrollBar()->sizeHint().width() - (hasTitle ? 24 : 4), lockButton->y());
        tabButton->move(lockButton->x()  - tabButton->width() - 4, lockButton->y());
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
        recreateLayout(currentWidgetMap);
    }
};

KoToolDocker::KoToolDocker(QWidget *parent)
    : QDockWidget(i18n("Tool Options"), parent),
    d(new Private(this))
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea);

    toggleViewAction()->setVisible(false); //should always be visible, so hide option in menu
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
    d->scrollArea->setWidgetResizable(true);

    setWidget(d->scrollArea);

    d->lockButton = new QToolButton(this);
    d->lockButton->setIcon(d->lockIcon);
    d->lockButton->setAutoRaise(true);
    connect(d->lockButton, SIGNAL(clicked()), SLOT(toggleLock()));
    d->lockButton->setVisible(true);
    d->lockButton->resize(d->lockButton->sizeHint());
    d->hasTitle = false;

    d->tabButton = new QToolButton(0); // parent hack in toggleLock to keep it clickable
    d->tabButton->setIcon(d->tabIcon);
    d->tabButton->setAutoRaise(true);
    connect(d->tabButton, SIGNAL(clicked()), SLOT(toggleTab()));
    d->tabButton->setVisible(false);
    d->tabButton->resize(d->tabButton->sizeHint());

    KConfigGroup cfg = KGlobal::config()->group("DockWidget sharedtooldocker");
    d->tabbed = cfg.readEntry("TabbedMode", false);
}

KoToolDocker::~KoToolDocker()
{
    KConfigGroup cfg = KGlobal::config()->group("DockWidget sharedtooldocker");
    cfg.writeEntry("TabbedMode", d->tabbed);

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

void KoToolDocker::resizeEvent(QResizeEvent*)
{
    int fw = isFloating() ? style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, this) : 0;
    int mw = style()->pixelMetric(QStyle::PM_DockWidgetTitleMargin, 0, this);
    QFontMetrics titleFontMetrics = fontMetrics();
    d->lockButton->move(width() - d->lockButton->width() - d->scrollArea->verticalScrollBar()->sizeHint().width() - (d->hasTitle ? 24 : 4), fw + mw);
    d->tabButton->move(d->lockButton->x()  - d->tabButton->width() - 4, d->lockButton->y());
}

#include <KoToolDocker_p.moc>
