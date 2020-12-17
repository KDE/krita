/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SPACING_SELECTION_WIDGET_H
#define __KIS_SPACING_SELECTION_WIDGET_H

#include <QWidget>
#include <QScopedPointer>

#include <kritapaintop_export.h>

class PAINTOP_EXPORT KisSpacingSelectionWidget : public QWidget
{
    Q_OBJECT
public:
    KisSpacingSelectionWidget(QWidget *parent);
    ~KisSpacingSelectionWidget() override;

    void setSpacing(bool isAuto, qreal spacing);

    qreal spacing() const;
    bool autoSpacingActive() const;
    qreal autoSpacingCoeff() const;

Q_SIGNALS:
    void sigSpacingChanged();

private:
    Q_PRIVATE_SLOT(m_d, void slotSpacingChanged(qreal value));
    Q_PRIVATE_SLOT(m_d, void slotAutoSpacing(bool value));

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_SPACING_SELECTION_WIDGET_H */
