/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "kis_utility_title_bar.h"
#include "kis_utility_title_bar_p.h"
#include <QDockWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include "KoDockWidgetTitleBarButton.h"
#include "qaction.h"
#include <klocalizedstring.h>
#include <KoIcon.h>
#include "kis_assert.h"


KisUtilityTitleBar::KisUtilityTitleBar(QWidget *parent)
    : KisUtilityTitleBar(new QLabel(i18n("Title"), parent), parent)
{
}

KisUtilityTitleBar::KisUtilityTitleBar(QLabel *title, QWidget *parent)
    : QWidget(parent), d(new Private(this))
{
    // Set up main layout..
    d->mainLayout = new QHBoxLayout(this);
    QMargins margins = d->mainLayout->contentsMargins();
    margins.setTop(0);
    margins.setBottom(0);
    margins.setLeft(0);
    d->mainLayout->setContentsMargins(margins);
    d->mainLayout->setSpacing(0);

    // Set up lock button
    d->lockButton = new KoDockWidgetTitleBarButton(this);
    d->lockButton->setCheckable(true);
    d->lockButton->setIcon(kisIcon("docker_lock_a"));
    connect(d->lockButton, SIGNAL(toggled(bool)), SLOT(setLocked(bool) ));
    d->lockButton->setVisible(true);
    d->lockButton->setToolTip(i18nc("@info:tooltip", "Lock Docker"));
    d->lockButton->setStyleSheet("border: 0");

    d->mainLayout->addWidget(d->lockButton);

    // Set up title..
    d->mainLayout->addWidget(title);

    //QSpacerItem *spacerItem = new QSpacerItem(SPACING_UNIT * 2, 0);
    d->mainLayout->addSpacing(SPACING_UNIT * 2);
    // Set up widget area..
    QWidget *widgetArea = new QWidget(this);
    widgetAreaLayout = new QHBoxLayout(widgetArea);
    widgetAreaLayout->setSpacing(0);
    widgetAreaLayout->setContentsMargins(0,0,0,0);
    d->mainLayout->addWidget(widgetArea);

//     mainLayout->addSpacing(SPACING_UNIT * 2);
    d->mainLayout->insertSpacing(4, SPACING_UNIT * 2 );

    {   // Controls/Decorations..
        QWidget *widget = new QWidget(this);

        QHBoxLayout *sublayout = new QHBoxLayout(widget);
        sublayout->setSpacing(0);
        sublayout->setContentsMargins(0,0,0,0);

        QDockWidget *dockWidget = qobject_cast<QDockWidget*>(parentWidget());

        {   // Float button...
            QIcon floatIcon = kisIcon("docker_float");
            d->floatButton = new QPushButton(floatIcon, "", this);
            d->floatButton->setFlat(true);
            connect(d->floatButton, &QPushButton::clicked, dockWidget, [dockWidget](){
                dockWidget->setFloating(!dockWidget->isFloating());
            } );
            sublayout->addWidget(d->floatButton);
        }

        {   // Close button...
            QIcon closeIcon = kisIcon("docker_close");
            d->closeButton = new QPushButton(closeIcon, "", this);
            d->closeButton->setFlat(true);
            connect(d->closeButton, SIGNAL(clicked(bool)), dockWidget, SLOT(close()));
            sublayout->addWidget(d->closeButton);
        }

        d->mainLayout->addWidget(widget);
    }
}

QWidget* KisUtilityTitleBar::widgetArea() {
    return widgetAreaLayout->parentWidget();
}

void KisUtilityTitleBar::setWidgetArea(QWidget* widgetArea) {
    QHBoxLayout *layout = dynamic_cast<QHBoxLayout*>(this->layout());
    KIS_SAFE_ASSERT_RECOVER_RETURN(layout);
    // It's the third widget;
    //     title, spacer, widgetArea, spacer, float/close
    layout->insertWidget(2, widgetArea);
}

void KisUtilityTitleBar::setLocked(bool locked)
{
    QDockWidget *q = qobject_cast<QDockWidget*>(parentWidget());
    d->locked = locked;
    d->lockButton->blockSignals(true);
    d->lockButton->setChecked(locked);
    d->lockButton->blockSignals(false);

    if (locked) {
        d->features = q->features();
        q->setFeatures(QDockWidget::NoDockWidgetFeatures);
        QLayoutItem *spacingItem = d->mainLayout->itemAt(4);
        d->mainLayout->removeItem(spacingItem);
    }
    else {
        d->mainLayout->insertSpacing(4, SPACING_UNIT * 2 );
        q->setFeatures(d->features);
    }

    q->toggleViewAction()->setEnabled(!locked);
    d->closeButton->setEnabled(!locked);
    d->closeButton->setVisible(!locked);

    d->floatButton->setEnabled(!locked);
    d->floatButton->setVisible(!locked);


    d->updateIcons();
    q->setProperty("Locked", locked);
}

void KisUtilityTitleBar::Private::updateIcons()
{
    lockIcon = (!locked) ? kisIcon("docker_lock_a") : kisIcon("docker_lock_b");
    lockButton->setIcon(lockIcon);

    // this method gets called when switching themes, so update all of the themed icons now
    floatButton->setIcon(kisIcon("docker_float"));
    closeButton->setIcon(kisIcon("docker_close"));

    thePublic->resizeEvent(0);
}

KisUtilityTitleBar::~KisUtilityTitleBar()
{
    delete d;
}
