/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisQmlPopupWidgetManager.h"
#include <KisQQuickPopupWidget.h>
#include <QApplication>

#include <optional>

struct KisQmlPopupWidgetManager::Private {
    KisQQuickPopupWidget *popup;

    qreal x;
    qreal y;

    qreal margins = 0;
    std::optional<qreal> topMargin = std::nullopt;
    std::optional<qreal> bottomMargin = std::nullopt;
    std::optional<qreal> leftMargin = std::nullopt;
    std::optional<qreal> rightMargin = std::nullopt;

    QQuickItem *itemParent;
};

KisQmlPopupWidgetManager::KisQmlPopupWidgetManager(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->popup = new KisQQuickPopupWidget();

    connect(this, SIGNAL(marginsChanged()), this, SLOT(updateMargins()));
    connect(this, SIGNAL(topMarginChanged()), this, SLOT(updateMargins()));
    connect(this, SIGNAL(leftMarginChanged()), this, SLOT(updateMargins()));
    connect(this, SIGNAL(rightMarginChanged()), this, SLOT(updateMargins()));
    connect(this, SIGNAL(bottomMarginChanged()), this, SLOT(updateMargins()));
    connect(d->popup, SIGNAL(signalRootObjectReady()), this, SIGNAL(rootControlChanged()));
}

KisQmlPopupWidgetManager::~KisQmlPopupWidgetManager()
{
    d->popup->deleteLater();
}

qreal KisQmlPopupWidgetManager::x() const
{
    return d->x;
}

void KisQmlPopupWidgetManager::setX(const qreal value)
{
    if (qFuzzyCompare(d->x, value)) return;
    d->x = value;
    Q_EMIT xChanged();
}

qreal KisQmlPopupWidgetManager::y() const
{
    return d->y;
}

void KisQmlPopupWidgetManager::setY(const qreal value)
{
    if (qFuzzyCompare(d->y, value)) return;
    d->y = value;
    Q_EMIT yChanged();
}

QQuickItem *KisQmlPopupWidgetManager::itemParent() const
{
    return d->itemParent;
}

void KisQmlPopupWidgetManager::setItemParent(QQuickItem *item)
{
    if(d->itemParent == item) return;
    d->itemParent = item;
    Q_EMIT itemParentChanged();
}

QQuickItem *KisQmlPopupWidgetManager::rootControl() const
{
    return d->popup->rootObject();
}

qreal KisQmlPopupWidgetManager::margins() const
{
    return d->margins;
}

void KisQmlPopupWidgetManager::setMargins(const qreal value)
{
    if (qFuzzyCompare(d->margins, value)) return;
    d->margins = value;
    Q_EMIT marginsChanged();
}

qreal KisQmlPopupWidgetManager::topMargin() const
{
    return (d->topMargin)? d->topMargin.value(): d->margins;
}

qreal KisQmlPopupWidgetManager::bottomMargin() const
{
    return (d->bottomMargin)? d->bottomMargin.value(): d->margins;
}

qreal KisQmlPopupWidgetManager::leftMargin() const
{
    return (d->leftMargin)? d->leftMargin.value(): d->margins;
}

qreal KisQmlPopupWidgetManager::rightMargin() const
{
    return (d->rightMargin)? d->rightMargin.value(): d->margins;
}

void KisQmlPopupWidgetManager::setTopMargin(const qreal value)
{
    if (d->topMargin) {
        if (qFuzzyCompare(d->topMargin.value(), value)) return;
    }
    d->topMargin = std::make_optional(value);
    Q_EMIT topMarginChanged();
}

void KisQmlPopupWidgetManager::setRightMargin(const qreal value)
{
    if (d->rightMargin) {
        if (qFuzzyCompare(d->rightMargin.value(), value)) return;
    }
    d->rightMargin = std::make_optional(value);
    Q_EMIT rightMarginChanged();
}

void KisQmlPopupWidgetManager::setLeftMargin(const qreal value)
{
    if (d->leftMargin) {
        if (qFuzzyCompare(d->leftMargin.value(), value)) return;
    }
    d->leftMargin = std::make_optional(value);
    Q_EMIT leftMarginChanged();
}

void KisQmlPopupWidgetManager::setBottomMargin(const qreal value)
{
    if (d->bottomMargin) {
        if (qFuzzyCompare(d->bottomMargin.value(), value)) return;
    }
    d->bottomMargin = std::make_optional(value);
    Q_EMIT bottomMarginChanged();
}

void KisQmlPopupWidgetManager::resetTopMargin()
{
    d->topMargin = std::nullopt;
}

void KisQmlPopupWidgetManager::resetLeftMargin()
{
    d->leftMargin = std::nullopt;
}

void KisQmlPopupWidgetManager::resetRightMargin()
{
    d->rightMargin = std::nullopt;
}

void KisQmlPopupWidgetManager::resetBottomMargin()
{
    d->bottomMargin = std::nullopt;
}

bool KisQmlPopupWidgetManager::visible() const
{
    return d->popup->isVisible();
}

void KisQmlPopupWidgetManager::open()
{
    if (!d->itemParent) {
        qWarning() << "Parent not set! PopupWidget requires parent to be explicitely set!";
        return;
    }
    d->popup->raise();
    d->popup->show();
    d->popup->move(d->itemParent->mapToGlobal(QPointF(d->x, d->y)).toPoint());
    // The following is probably causing issues with keyboard focus :(
    d->popup->activateWindow();
    Q_EMIT visibleChanged();
}

void KisQmlPopupWidgetManager::close()
{
    d->popup->hide();
    Q_EMIT visibleChanged();
}

void KisQmlPopupWidgetManager::releaseKeyboard()
{
    d->popup->releaseKeyboard();
}

void KisQmlPopupWidgetManager::updateMargins()
{
    QMargins margins;
    margins.setTop(qRound(topMargin()));
    margins.setLeft(qRound(leftMargin()));
    margins.setRight(qRound(rightMargin()));
    margins.setBottom(qRound(bottomMargin()));
    d->popup->setLayoutMargins(margins);
}
