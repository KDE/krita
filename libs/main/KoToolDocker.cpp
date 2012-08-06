/* This file is part of the KDE project
 *
 * Copyright (c) 2010-2011 C. Boemann <cbo@boemann.dk>
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
#include <KoIcon.h>

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
#include <QToolButton>

class KoToolDocker::Private {
public:
    Private(KoToolDocker *dock)
            : q(dock)
            , tabbed(false)
            , tabIcon(koIconName("tab-new"))
            , unTabIcon(koIconName("tab-close"))
    {
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
    KIcon tabIcon;
    KIcon unTabIcon;
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

        // need to unstretch row that have previously been stretched
        housekeeperLayout->setRowStretch(housekeeperLayout->rowCount()-1, 0);

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
            case Qt::RightDockWidgetArea: {
                housekeeperLayout->setHorizontalSpacing(0);
                housekeeperLayout->setVerticalSpacing(2);
                int specialCount = 0;
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
                    QLayout *subLayout = widget->layout();
                    if (subLayout) {
                        for (int i = 0; i < subLayout->count(); ++i) {
                            QWidget *spacerWidget = subLayout->itemAt(i)->widget();
                            if (spacerWidget && spacerWidget->objectName().contains("SpecialSpacer")) {
                                specialCount++;
                            }
                        }
                    }
                    widget->show();
                    if (widget != currentWidgetList.last()) {
                        housekeeperLayout->addWidget(s = new QFrame(), cnt++, 0);
                        s->setFrameShape(QFrame::HLine);
                        currentAuxWidgets.insert(s);
                    }
                }
                if (specialCount == currentWidgetList.count()) {
                    housekeeperLayout->setRowStretch(cnt, 100);
                }
                break;
            }
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

    toggleViewAction()->setVisible(false); //should always be visible, so hide option in menu
    setFeatures(DockWidgetMovable|DockWidgetFloatable);
    setTitleBarWidget(new KoDockWidgetTitleBar(this));

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
    d->scrollArea->setFocusPolicy(Qt::NoFocus);

    setWidget(d->scrollArea);

    d->tabButton = new QToolButton(this); // parent hack in toggleLock to keep it clickable
    d->tabButton->setIcon(d->tabIcon);
    d->tabButton->setToolTip(i18n("Toggles organising the options in tabs or not"));
    d->tabButton->setAutoRaise(true);
    connect(d->tabButton, SIGNAL(clicked()), SLOT(toggleTab()));
    d->tabButton->resize(d->tabButton->sizeHint());
}

KoToolDocker::~KoToolDocker()
{
    KConfigGroup cfg = KGlobal::config()->group("DockWidget sharedtooldocker");
    cfg.writeEntry("TabbedMode", d->tabbed);
    cfg.sync();

    delete d;
}

bool KoToolDocker::hasOptionWidget()
{
    return !d->currentWidgetList.isEmpty();
}

void KoToolDocker::setTabEnabled(bool enabled)
{
    d->tabButton->setVisible(enabled);
}

void KoToolDocker::setOptionWidgets(const QList<QWidget *> &optionWidgetList)
{
    d->recreateLayout(optionWidgetList);
}

void KoToolDocker::resizeEvent(QResizeEvent*)
{
    int fw = isFloating() ? style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, this) : 0;
    d->tabButton->move(width() - d->tabButton->width() - d->scrollArea->verticalScrollBar()->sizeHint().width(), fw);
}

void KoToolDocker::resetWidgets()
{
    d->resetWidgets();
}

#include <KoToolDocker_p.moc>
