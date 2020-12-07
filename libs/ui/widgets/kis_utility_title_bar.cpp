/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "kis_utility_title_bar.h"

#include <QDockWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>

#include <klocalizedstring.h>


KisUtilityTitleBar::KisUtilityTitleBar(QWidget *parent)
    : KisUtilityTitleBar(new QLabel(i18n("Title"), parent), parent)
{
}

KisUtilityTitleBar::KisUtilityTitleBar(QLabel *title, QWidget *parent)
    : QWidget(parent)
{
    // Set up main layout..
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    QMargins margins = mainLayout->contentsMargins();
    margins.setTop(0);
    margins.setBottom(0);
    mainLayout->setContentsMargins(margins);
    mainLayout->setSpacing(0);

    // Set up title..
    mainLayout->addWidget(title);

    mainLayout->addSpacing(SPACING_UNIT * 2);

    // Set up widget area..
    QWidget *widgetArea = new QWidget(this);
    widgetAreaLayout = new QHBoxLayout(widgetArea);
    widgetAreaLayout->setSpacing(0);
    widgetAreaLayout->setContentsMargins(0,0,0,0);
    mainLayout->addWidget(widgetArea);

    mainLayout->addSpacing(SPACING_UNIT * 2);

    {   // Controls/Decorations..
        QWidget *widget = new QWidget(this);

        QHBoxLayout *sublayout = new QHBoxLayout(widget);
        sublayout->setSpacing(0);
        sublayout->setContentsMargins(0,0,0,0);

        QDockWidget *dockWidget = qobject_cast<QDockWidget*>(parentWidget());

        {   // Float button...
            QPushButton *button = new QPushButton(style()->standardIcon(QStyle::SP_TitleBarNormalButton), "", this);
            button->setFlat(true);
            connect(button, &QPushButton::clicked, [dockWidget](){
                dockWidget->setFloating(!dockWidget->isFloating());
            } );
            sublayout->addWidget(button);
        }

        {   // Close button...
            QPushButton *button = new QPushButton(style()->standardIcon(QStyle::SP_DockWidgetCloseButton), "", this);
            button->setFlat(true);
            connect(button, SIGNAL(clicked(bool)), dockWidget, SLOT(close()));
            sublayout->addWidget(button);
        }

        mainLayout->addWidget(widget);
    }
}
