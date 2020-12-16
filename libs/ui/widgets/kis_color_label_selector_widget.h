/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_COLOR_LABEL_SELECTOR_WIDGET_H
#define __KIS_COLOR_LABEL_SELECTOR_WIDGET_H

#include <QScopedPointer>
#include <QWidget>

#include "kritaui_export.h"


class KRITAUI_EXPORT KisColorLabelSelectorWidget : public QWidget
{
    Q_OBJECT

public:
    KisColorLabelSelectorWidget(QWidget *parent);
    ~KisColorLabelSelectorWidget() override;

    int currentIndex() const;

    QSize sizeHint() const override;
    void resizeEvent(QResizeEvent* e) override;

    int calculateMenuOffset() const;

public Q_SLOTS:
    void groupButtonChecked(int index, bool state);
    void setCurrentIndex(int index);

Q_SIGNALS:
    void currentIndexChanged(int index);

private:
    struct Private* m_d;
};

#endif /* __KIS_COLOR_LABEL_SELECTOR_WIDGET_H */
