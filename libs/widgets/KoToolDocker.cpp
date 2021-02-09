/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2010-2011 C. Boemann <cbo@boemann.dk>
 * SPDX-FileCopyrightText: 2005-2006 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KoToolDocker.h"

#include <klocalizedstring.h>

#include <QApplication>
#include <QGridLayout>
#include <QScrollArea>
#include <QScroller>
#include <QLabel>

class Q_DECL_HIDDEN KoToolDocker::Private
{
public:
    Private(KoToolDocker *dock)
        : q(dock)
    {
    }

    QList<QPointer<QWidget> > currentWidgetList;
    QSet<QWidget *> currentAuxWidgets;
    QScrollArea *scrollArea;
    QWidget *hiderWidget; // non current widgets are hidden by being children of this
    QWidget *housekeeperWidget;
    QGridLayout *housekeeperLayout;
    KoToolDocker *q;
    Qt::DockWidgetArea dockingArea;

    void resetWidgets()
    {
        currentWidgetList.clear();
        qDeleteAll(currentAuxWidgets);
        currentAuxWidgets.clear();
    }

    void recreateLayout(const QList<QPointer<QWidget> > &optionWidgetList)
    {
        Q_FOREACH (QPointer<QWidget> widget, currentWidgetList) {
            if (!widget.isNull() && widget && hiderWidget) {
                widget->setParent(hiderWidget);
            }
        }
        qDeleteAll(currentAuxWidgets);
        currentAuxWidgets.clear();

        currentWidgetList = optionWidgetList;

        // need to unstretch row that have previously been stretched
        housekeeperLayout->setRowStretch(housekeeperLayout->rowCount()-1, 0);

            int cnt = 0;
            QFrame *s;
            QLabel *l;
            switch(dockingArea) {
            case Qt::TopDockWidgetArea:
            case Qt::BottomDockWidgetArea:
                housekeeperLayout->setHorizontalSpacing(2);
                housekeeperLayout->setVerticalSpacing(0);
                Q_FOREACH (QPointer<QWidget> widget, currentWidgetList) {
                    if (widget.isNull() || widget->objectName().isEmpty()) {
                        continue;
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
            case Qt::NoDockWidgetArea:
            case Qt::LeftDockWidgetArea:
            case Qt::RightDockWidgetArea: {
                housekeeperLayout->setHorizontalSpacing(0);
                housekeeperLayout->setVerticalSpacing(2);
                int specialCount = 0;
                Q_FOREACH (QPointer<QWidget> widget, currentWidgetList) {
                    if (widget.isNull() || widget->objectName().isEmpty()) {
                        continue;
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
                        s->setFrameStyle(QFrame::HLine | QFrame::Sunken);
                        currentAuxWidgets.insert(s);
                    }
                }
                if (specialCount == currentWidgetList.count() || qApp->applicationName().contains("krita")) {
                    housekeeperLayout->setRowStretch(cnt, 10000);
                }
                break;
            }
            default:
                break;
            }

        housekeeperLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        housekeeperLayout->invalidate();
    }

    void locationChanged(Qt::DockWidgetArea area)
    {
        dockingArea = area;
        recreateLayout(currentWidgetList);
    }

};

KoToolDocker::KoToolDocker(QWidget *parent)
    : QDockWidget(i18n("Tool Options"), parent),
      d(new Private(this))
{
    connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(locationChanged(Qt::DockWidgetArea)));

    d->housekeeperWidget = new QWidget();
    d->housekeeperLayout = new QGridLayout(d->housekeeperWidget);
    d->housekeeperLayout->setContentsMargins(4,4,4,0);
    d->housekeeperLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    d->hiderWidget = new QWidget(d->housekeeperWidget);
    d->hiderWidget->setVisible(false);

    d->scrollArea = new QScrollArea();
    d->scrollArea->setWidget(d->housekeeperWidget);
    d->scrollArea->setFrameShape(QFrame::NoFrame);
    d->scrollArea->setWidgetResizable(true);
    d->scrollArea->setFocusPolicy(Qt::NoFocus);

    QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(d->scrollArea);
    if( scroller ) {
        connect(scroller, SIGNAL(stateChanged(QScroller::State)), this, SLOT(slotScrollerStateChange(QScroller::State)));
    }

    setWidget(d->scrollArea);
}

KoToolDocker::~KoToolDocker()
{
    delete d;
}

bool KoToolDocker::hasOptionWidget()
{
    return !d->currentWidgetList.isEmpty();
}

void KoToolDocker::setOptionWidgets(const QList<QPointer<QWidget> > &optionWidgetList)
{
    d->recreateLayout(optionWidgetList);
}

void KoToolDocker::slotScrollerStateChange(QScroller::State state)
{
    KisKineticScroller::updateCursor(d->scrollArea, state);
}

void KoToolDocker::resetWidgets()
{
    d->resetWidgets();
}


void KoToolDocker::setCanvas(KoCanvasBase *canvas)
{
    setEnabled(canvas != 0);
}

void KoToolDocker::unsetCanvas()
{
    setEnabled(false);
}

//have to include this because of Q_PRIVATE_SLOT
#include <moc_KoToolDocker.cpp>
