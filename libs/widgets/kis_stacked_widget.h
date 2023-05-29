// SPDX-FileCopyrightText: 2023 L. E. Segovia <amy@amyspark.me>
// SPDX-License-Ref: LGPL-2.0-or-later

#ifndef KIS_STACKED_WIDGET_H
#define KIS_STACKED_WIDGET_H

#include <QStackedWidget>

#include "kritawidgets_export.h"

/** This class enables a @ref QStackedWidget to resize itself to its current
 * visible member's @ref sizeHint, instead of staying with the maximum of all
 * children's sizes.
 * This class is intended to be used in place of @ref QStackedWidget, whenever
 * the latter is meant to be the main widget.
 */
class KRITAWIDGETS_EXPORT KisStackedWidget : public QStackedWidget
{
public:
    KisStackedWidget(QWidget *parent = nullptr);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
    Q_DISABLE_COPY_MOVE(KisStackedWidget);
#else
    //Q_DISABLE_MOVE(KisStackedWidget);
    Q_DISABLE_COPY(KisStackedWidget)
    KisStackedWidget(KisStackedWidget&&) = delete;
    KisStackedWidget& operator=(KisStackedWidget &&) = delete;
#endif

    ~KisStackedWidget() override = default;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
};

#endif
