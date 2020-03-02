/*
 * Copyright (c) 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
