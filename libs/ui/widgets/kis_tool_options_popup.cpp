/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "widgets/kis_tool_options_popup.h"


#include <QList>
#include <QFont>
#include <QMenu>
#include <QAction>
#include <QShowEvent>
#include <QPointer>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>
#include <QApplication>
#include <QFontDatabase>

#include <KoDockRegistry.h>

#include <kconfig.h>
#include <klocalizedstring.h>


#include "kis_config.h"

struct KisToolOptionsPopup::Private
{
public:
    QFont smallFont;
    bool ignoreHideEvents;

    QList<QPointer<QWidget> > currentWidgetList;
    QSet<QWidget *> currentAuxWidgets;
    QWidget *hiderWidget; // non current widgets are hidden by being children of this
    QGridLayout *housekeeperLayout;

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
        housekeeperLayout->setHorizontalSpacing(0);
        housekeeperLayout->setVerticalSpacing(2);
        int specialCount = 0;
        Q_FOREACH (QPointer<QWidget> widget, currentWidgetList) {
            if (widget.isNull() || widget->objectName().isEmpty()) {
                continue; // skip this docker in release build when assert don't crash
            }

            widget->setMinimumWidth(300);

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

        housekeeperLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        housekeeperLayout->invalidate();
    }

};


KisToolOptionsPopup::KisToolOptionsPopup(QWidget *parent)
    : QWidget(parent)
    , d(new Private())
{
    setObjectName("KisToolOptionsPopup");

    KConfigGroup group( KSharedConfig::openConfig(), "GUI");
    setFont(KoDockRegistry::dockFont());

    d->ignoreHideEvents = false;

    d->housekeeperLayout = new QGridLayout(this);
    d->housekeeperLayout->setContentsMargins(4,4,4,0);
    d->housekeeperLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    d->hiderWidget = new QWidget(this);
    d->hiderWidget->setVisible(false);
}


KisToolOptionsPopup::~KisToolOptionsPopup()
{
    delete d;
}

void KisToolOptionsPopup::newOptionWidgets(const QList<QPointer<QWidget> > &optionWidgetList)
{
    d->recreateLayout(optionWidgetList);
}

void KisToolOptionsPopup::contextMenuEvent(QContextMenuEvent *e) {

    Q_UNUSED(e);
}

void KisToolOptionsPopup::hideEvent(QHideEvent *event)
{
    if (d->ignoreHideEvents) {
        return;
    }
    QWidget::hideEvent(event);
}

void KisToolOptionsPopup::showEvent(QShowEvent *)
{
}
