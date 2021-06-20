/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
   SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>

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
#include <QLabel>
#include <QLayout>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOptionFrame>

static inline bool hasFeature(const QDockWidget *dockwidget, QDockWidget::DockWidgetFeature feature)
{
    return (dockwidget->features() & feature) == feature;
}

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
    d->lockable = true;
    d->lockButton->setToolTip(i18nc("@info:tooltip", "Lock Docker"));
    d->lockButton->setStyleSheet("border: 0");

    connect(dockWidget, SIGNAL(featuresChanged(QDockWidget::DockWidgetFeatures)), SLOT(featuresChanged(QDockWidget::DockWidgetFeatures)));
    connect(dockWidget, SIGNAL(topLevelChanged(bool)), SLOT(topLevelChanged(bool)));

    d->featuresChanged(QDockWidget::NoDockWidgetFeatures);
}

KoDockWidgetTitleBar::~KoDockWidgetTitleBar()
{
    delete d;
}

QSize KoDockWidgetTitleBar::minimumSizeHint() const
{
    return sizeHint();
}

QSize KoDockWidgetTitleBar::sizeHint() const
{
    if (isHidden()) {
        return QSize(0, 0);
    }

    QDockWidget *q = qobject_cast<QDockWidget*>(parentWidget());

    int mw = q->style()->pixelMetric(QStyle::PM_DockWidgetTitleMargin, 0, q);
    int fw = q->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, q);

    // get size of buttons...
    QSize closeSize(0, 0);
    if (d->closeButton && hasFeature(q, QDockWidget::DockWidgetClosable)) {
        closeSize = d->closeButton->sizeHint();
    }

    QSize floatSize(0, 0);
    if (d->floatButton && hasFeature(q, QDockWidget::DockWidgetFloatable)) {
        floatSize = d->floatButton->sizeHint();
    }

    QSize lockSize(0, 0);
    if (d->lockButton && d->lockable) {
        lockSize = d->lockButton->sizeHint();
    }

    int buttonHeight = qMax(qMax(closeSize.height(), floatSize.height()), lockSize.height()) + 2;
    int buttonWidth = closeSize.width() + lockSize.width() + floatSize.width();

    int height = buttonHeight;
    if (d->textVisibilityMode == FullTextAlwaysVisible) {
        // get font size
        QFontMetrics titleFontMetrics = q->fontMetrics();
        int fontHeight = titleFontMetrics.lineSpacing() + 2 * mw;

        height = qMax(height, fontHeight);
    }

    if (d->textVisibilityMode == FullTextAlwaysVisible) {
        int titleWidth = 0;
#if QT_VERSION >= QT_VERSION_CHECK(5,11,0)
        titleWidth = q->fontMetrics().horizontalAdvance(q->windowTitle()) + 2 * mw;
#else
        titleWidth = q->fontMetrics().width(q->windowTitle()) + 2 * mw;
#endif



        return QSize(buttonWidth /*+ height*/ + 2*mw + 2*fw + titleWidth, height);
    } else if (q->widget()) {
        return QSize(qMin(q->widget()->minimumSizeHint().width(), buttonWidth), height);
    } else {
        return QSize(buttonWidth, height);
    }
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
    if (d->lockable && d->lockButton->isVisible()) {
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
    titleOpt.title = q->windowTitle();
    titleOpt.closable = hasFeature(q, QDockWidget::DockWidgetClosable);
    titleOpt.floatable = hasFeature(q, QDockWidget::DockWidgetFloatable);
    p.drawControl(QStyle::CE_DockWidgetTitle, titleOpt);
}

void KoDockWidgetTitleBar::resizeEvent(QResizeEvent*)
{
    QDockWidget *q = qobject_cast<QDockWidget*>(parentWidget());

    int fw = q->isFloating() ? q->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, q) : 0;

    QStyleOptionDockWidget opt;
    opt.initFrom(q);
    opt.rect = QRect(QPoint(fw, fw), QSize(geometry().width() - (fw * 2), geometry().height() - (fw * 2)));
    opt.title = q->windowTitle();
    opt.closable = hasFeature(q, QDockWidget::DockWidgetClosable);
    opt.floatable = hasFeature(q, QDockWidget::DockWidgetFloatable);

    QRect floatRect = q->style()->subElementRect(QStyle::SE_DockWidgetFloatButton, &opt, q);
    // To improve the look with Fusion which has weird 13x15 button sizes
    bool styleIsFusion = false;
    QSize overrideSize = QSize(16, 16);
    if (floatRect.width() < 16) {
        styleIsFusion = true;
        floatRect.setSize(overrideSize);
        floatRect.moveLeft(floatRect.x() - 15);
    }

    if (!floatRect.isNull())
        d->floatButton->setGeometry(floatRect);

    QRect closeRect = q->style()->subElementRect(QStyle::SE_DockWidgetCloseButton, &opt, q);
    // To improve the look with Fusion which has weird 13x15 button sizes
    if (styleIsFusion) {
        closeRect.setSize(overrideSize);
        closeRect.moveLeft(closeRect.x() - 6);
    }

    if (!closeRect.isNull())
        d->closeButton->setGeometry(closeRect);

    QSize lockRectSize = d->lockButton->size();
    if (!closeRect.isNull()) {
        lockRectSize = d->closeButton->size();
    } else if (!floatRect.isNull()) {
        lockRectSize = d->floatButton->size();
    }

    if (q->isFloating() || (width() < (closeRect.width() + lockRectSize.width()) + 50)) {
        d->lockButton->setVisible(false);
        d->lockable = false;
    } else {
        d->lockButton->setVisible(true);
        d->lockable = true;
    }

    int offset = 0;
    // To improve the look with Fusion which has weird 13x15 button sizes
    if (styleIsFusion) {
        offset = 3;
    }

    // Just centre the button vertically to prevent the button from shifting
    // up and down when toggling due to the close and float buttons changing
    // visibility.
    int top = (height() - lockRectSize.height()) * 0.5;

    QRect lockRect = QRect(QPoint(fw + 2 + offset, top), lockRectSize);
    d->lockButton->setGeometry(lockRect);
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


void KoDockWidgetTitleBar::setTextVisibilityMode(TextVisibilityMode textVisibilityMode)
{
    d->textVisibilityMode = textVisibilityMode;
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

    thePublic->resizeEvent(0);
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

//have to include this because of Q_PRIVATE_SLOT
#include "moc_KoDockWidgetTitleBar.cpp"
