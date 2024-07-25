// SPDX-FileCopyrightText: 2023 L. E. Segovia <amy@amyspark.me>
// SPDX-License-Ref: LGPL-2.0-or-later

#include "kis_stacked_widget.h"

KisStackedWidget::KisStackedWidget(QWidget *parent)
    : QStackedWidget(parent)
{
}

QSize KisStackedWidget::sizeHint() const
{
    const QWidget *wdg = currentWidget();
    if (wdg != nullptr) {
        return wdg->minimumSizeHint();
    }
    return QStackedWidget::minimumSize();
}

QSize KisStackedWidget::minimumSizeHint() const
{
    const QWidget *wdg = currentWidget();
    if (wdg != nullptr) {
        return wdg->minimumSizeHint();
    }
    return QStackedWidget::minimumSizeHint();
}
