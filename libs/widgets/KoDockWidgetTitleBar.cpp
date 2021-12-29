/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
   SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
   SPDX-FileCopyrightText: 2021 Alvin Wong <alvin@alvinhc.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KoDockWidgetTitleBar.h"
#include "KoDockWidgetTitleBar_p.h"
#include "KoDockWidgetTitleBarButton.h"

#include <KoIcon.h>
#include <kis_icon_utils.h>

#include <WidgetsDebug.h>
#include <klocalizedstring.h>

#include <QAbstractButton>
#include <QAction>
#include <QHBoxLayout>
#include <QLabel>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOptionFrame>

#include <KSqueezedTextLabel>

static inline bool hasFeature(const QDockWidget *dockwidget, QDockWidget::DockWidgetFeature feature)
{
    return (dockwidget->features() & feature) == feature;
}

constexpr int SPACING = 6;

KoDockWidgetTitleBar::KoDockWidgetTitleBar(QDockWidget* dockWidget)
        : QWidget(dockWidget), d(new Private(this))
{
    d->floatIcon = kisIcon("docker_float");
    d->floatButton = new KoDockWidgetTitleBarButton(this);
    d->floatButton->setIcon(d->floatIcon);
    connect(d->floatButton, SIGNAL(clicked()), SLOT(toggleFloating()));
    d->floatButton->setVisible(true);
    d->floatButton->setToolTip(i18nc("@info:tooltip", "Float Docker"));
    d->floatButton->setStyleSheet("border: 0");

    d->removeIcon = kisIcon("docker_close");
    d->closeButton = new KoDockWidgetTitleBarButton(this);
    d->closeButton->setIcon(d->removeIcon);
    connect(d->closeButton, SIGNAL(clicked()), dockWidget, SLOT(close()));
    d->closeButton->setVisible(true);
    d->closeButton->setToolTip(i18nc("@info:tooltip", "Close Docker"));   
    d->closeButton->setStyleSheet("border: 0"); // border makes the header busy looking (appears on some OSs)

    d->lockIcon = kisIcon("docker_lock_a");
    d->lockButton = new KoDockWidgetTitleBarButton(this);
    d->lockButton->setCheckable(true);
    d->lockButton->setIcon(d->lockIcon);
    connect(d->lockButton, SIGNAL(toggled(bool)), SLOT(setLocked(bool)));
    d->lockButton->setVisible(true);
    d->lockButton->setToolTip(i18nc("@info:tooltip", "Lock Docker"));
    d->lockButton->setStyleSheet("border: 0");

    d->updateButtonSizes();

    d->titleLabel = new KSqueezedTextLabel(this);
    d->titleLabel->setTextElideMode(Qt::ElideRight);
    d->titleLabel->setText(dockWidget->windowTitle());

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 0, 2, 0);
    layout->setSpacing(SPACING);
    layout->addWidget(d->lockButton);
    layout->addWidget(d->titleLabel, 1);
    layout->addWidget(d->floatButton);
    layout->addWidget(d->closeButton);

    connect(dockWidget, SIGNAL(featuresChanged(QDockWidget::DockWidgetFeatures)), SLOT(featuresChanged(QDockWidget::DockWidgetFeatures)));
    connect(dockWidget, SIGNAL(topLevelChanged(bool)), SLOT(topLevelChanged(bool)));
    connect(dockWidget, SIGNAL(windowTitleChanged(const QString &)), SLOT(dockWidgetTitleChanged(const QString &)));

    d->featuresChanged(QDockWidget::NoDockWidgetFeatures);
}

KoDockWidgetTitleBar::~KoDockWidgetTitleBar()
{
    delete d;
}

void KoDockWidgetTitleBar::paintEvent(QPaintEvent*)
{
    QStylePainter p(this);

    QDockWidget *q = qobject_cast<QDockWidget*>(parentWidget());

    int fw = q->isFloating() ? q->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, q) : 0;
    int mw = q->style()->pixelMetric(QStyle::PM_DockWidgetTitleMargin, 0, q);

    QStyleOptionDockWidget titleOpt;
    titleOpt.initFrom(q);

    QSize lockButtonSize(0,0);
    if (d->lockButton->isVisible()) {
        lockButtonSize = d->lockButton->size();
    }

    // To improve the look with Fusion which has weird 13x15 button sizes
    int fusionTextOffset = 0;
    QRect styleTestRect = q->style()->subElementRect(QStyle::SE_DockWidgetFloatButton, &titleOpt, q);
    if (styleTestRect.width() < 16) {
        fusionTextOffset = d->lockButton->x();
    }

    titleOpt.rect = QRect(QPoint(fw + mw + lockButtonSize.width() + fusionTextOffset, 0),
                          QSize(geometry().width() - (fw * 2) -  mw - lockButtonSize.width(), geometry().height()));
    // We don't print the title text here. Instead, we have a QLabel.
    //   titleOpt.title = q->windowTitle();
    // FIXME: Maybe we just shouldn't use a QStylePainter at all?
    titleOpt.title = QString();
    titleOpt.closable = hasFeature(q, QDockWidget::DockWidgetClosable);
    titleOpt.floatable = hasFeature(q, QDockWidget::DockWidgetFloatable);
    p.drawControl(QStyle::CE_DockWidgetTitle, titleOpt);
}

void KoDockWidgetTitleBar::resizeEvent(QResizeEvent*)
{
    QDockWidget *q = qobject_cast<QDockWidget*>(parentWidget());

    if (q->isFloating() || (width() < (d->closeButton->width() + d->floatButton->width() + d->lockButton->width()) + 32)) {
        d->lockButton->setVisible(false);
    } else {
        d->lockButton->setVisible(true);
    }
}

void KoDockWidgetTitleBar::setLocked(bool locked)
{
    QDockWidget *q = qobject_cast<QDockWidget*>(parentWidget());

    d->locked = locked;
    d->lockButton->blockSignals(true);
    d->lockButton->setChecked(locked);
    d->lockButton->blockSignals(false);

    if (locked) {
        d->features = q->features();
        q->setFeatures(QDockWidget::NoDockWidgetFeatures);
    }
    else {
        q->setFeatures(d->features);
    }

    q->toggleViewAction()->setEnabled(!locked);
    d->closeButton->setEnabled(!locked);
    d->floatButton->setEnabled(!locked);
    d->floatButton->setVisible(!locked);

    d->updateIcons();
    q->setProperty("Locked", locked);
}

void KoDockWidgetTitleBar::updateIcons()
{
    d->updateIcons();
}

void KoDockWidgetTitleBar::Private::toggleFloating()
{
    QDockWidget *q = qobject_cast<QDockWidget*>(thePublic->parentWidget());

    q->setFloating(!q->isFloating());
    updateIcons();
}

void KoDockWidgetTitleBar::Private::topLevelChanged(bool topLevel)
{
    lockButton->setEnabled(!topLevel);
    updateIcons();
}

void KoDockWidgetTitleBar::Private::featuresChanged(QDockWidget::DockWidgetFeatures)
{
    QDockWidget *q = qobject_cast<QDockWidget*>(thePublic->parentWidget());

    closeButton->setVisible(hasFeature(q, QDockWidget::DockWidgetClosable));
    floatButton->setVisible(hasFeature(q, QDockWidget::DockWidgetFloatable));

    updateButtonSizes();
    thePublic->resizeEvent(0);
}

void KoDockWidgetTitleBar::Private::dockWidgetTitleChanged(const QString &title)
{
    titleLabel->setText(title);
}


void KoDockWidgetTitleBar::Private::updateIcons()
{
    lockIcon = (!locked) ? kisIcon("docker_lock_a") : kisIcon("docker_lock_b");
    lockButton->setIcon(lockIcon);

    // this method gets called when switching themes, so update all of the themed icons now
    floatButton->setIcon(kisIcon("docker_float"));
    closeButton->setIcon(kisIcon("docker_close"));

    thePublic->resizeEvent(0);
}

void KoDockWidgetTitleBar::Private::updateButtonSizes()
{
    const QDockWidget *q = qobject_cast<QDockWidget*>(thePublic->parentWidget());

    const int fw = q->isFloating() ? q->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, q) : 0;

    QStyleOptionDockWidget opt;
    opt.initFrom(q);
    opt.rect = QRect(QPoint(fw, fw), QSize(thePublic->geometry().width() - (fw * 2), thePublic->geometry().height() - (fw * 2)));
    opt.title = q->windowTitle();
    // Originally it was:
    //   opt.closable = hasFeature(q, QDockWidget::DockWidgetClosable);
    // but I think we better just always pretend the close button is visible to
    // get the button size.
    opt.closable = true;
    opt.floatable = hasFeature(q, QDockWidget::DockWidgetFloatable);

    // Again, we just always use the size of the close button, so we don't
    // need to get the size of the float button...
    //   QRect floatRect = q->style()->subElementRect(QStyle::SE_DockWidgetFloatButton, &opt, q);
    const QRect closeRect = q->style()->subElementRect(QStyle::SE_DockWidgetCloseButton, &opt, q);
    QSize buttonSize = closeRect.size();
    if (buttonSize.width() < 16) {
        // To improve the look with Fusion which has weird 13x15 button sizes
        buttonSize = QSize(16, 16);
    } else if (buttonSize.width() != buttonSize.height()) {
        // Just make sure the button is square...
        buttonSize.setHeight(buttonSize.width());
    }

    floatButton->setFixedSize(buttonSize);
    closeButton->setFixedSize(buttonSize);
    lockButton->setFixedSize(buttonSize);
}

//have to include this because of Q_PRIVATE_SLOT
#include "moc_KoDockWidgetTitleBar.cpp"
