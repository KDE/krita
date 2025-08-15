/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISQQUICKWIDGET_H
#define KISQQUICKWIDGET_H

#include <QQuickWidget>
#include <kritaqmlwidgets_export.h>

/**
 * @brief The KisQQuickWidget class
 *
 * The purpose of KisQQuickWidget is to tackle a number of configuration steps
 * that need to be taken when using the QQuickWidget, amongst which the setup
 * of the engine.
 */

class KRITAQMLWIDGETS_EXPORT KisQQuickWidget : public QQuickWidget
{
    Q_OBJECT
public:
    KisQQuickWidget(QWidget *parent = nullptr);

    ~KisQQuickWidget();
};

#endif // KISQQUICKWIDGET_H
