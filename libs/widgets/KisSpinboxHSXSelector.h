/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSPINBOXHSXSELECTOR_H
#define KISSPINBOXHSXSELECTOR_H

#include <QWidget>
#include <QVector3D>
#include "kritawidgets_export.h"
#include <QScopedPointer>

class KisVisualColorSelector;

/**
 * A set of spinboxes to adjust HSV, HSL, HSI or HSY' values.
 * This class is meant to be used in conjunction with KisVisualColorSelector only.
 */

class KRITAWIDGETS_EXPORT KisSpinboxHSXSelector : public QWidget
{
    Q_OBJECT
public:
    explicit KisSpinboxHSXSelector(QWidget *parent = nullptr);
    ~KisSpinboxHSXSelector() override;

    /**
     * @brief connect signals and slots with given selector to
     *        synchronize value and color model changes.
     *
     * Note: in case the selector switches to a non-HSX color model,
     * it needs to be deactivated or hidden manually since it will
     * not synchronize values for other selector models.
     */
    void attachToSelector(KisVisualColorSelector *selector);

Q_SIGNALS:
    void sigHSXChanged(const QVector3D &hsx);

public Q_SLOTS:
    void slotColorModelChanged();
    void slotHSXChanged(const QVector3D &hsx);
private Q_SLOTS:
    void slotSpinBoxChanged();
private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISSPINBOXHSXSELECTOR_H
