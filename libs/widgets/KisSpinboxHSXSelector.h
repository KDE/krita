/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSPINBOXHSXSELECTOR_H
#define KISSPINBOXHSXSELECTOR_H

#include "kritawidgets_export.h"
#include "KisVisualColorModel.h"

#include <QWidget>
#include <QVector4D>
#include <QScopedPointer>

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
     * @brief connect signals and slots with given selector model
     *        to synchronize value and color model changes.
     *
     * Note: in case the selector switches to a non-HSX color model,
     * it needs to be deactivated or hidden manually since it will
     * not synchronize values for other selector models.
     */
    void setModel(KisVisualColorModelSP model);

Q_SIGNALS:
    void sigChannelValuesChanged(const QVector4D &values);

public Q_SLOTS:
    void slotColorModelChanged();
    void slotChannelValuesChanged(const QVector4D &values);
private Q_SLOTS:
    void slotSpinBoxChanged();
private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISSPINBOXHSXSELECTOR_H
