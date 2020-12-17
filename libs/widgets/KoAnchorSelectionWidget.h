/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOANCHORSELECTIONWIDGET_H
#define KOANCHORSELECTIONWIDGET_H

#include <QWidget>
#include <KoFlake.h>

#include "kritawidgets_export.h"


class KRITAWIDGETS_EXPORT KoAnchorSelectionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KoAnchorSelectionWidget(QWidget *parent = 0);
    ~KoAnchorSelectionWidget() override;

    KoFlake::AnchorPosition value() const;
    QPointF value(const QRectF rect, bool *valid) const;

    void setValue(KoFlake::AnchorPosition value);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void valueChanged(KoFlake::AnchorPosition id);

public Q_SLOTS:
    void slotGroupClicked(int id);
private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KOANCHORSELECTIONWIDGET_H
