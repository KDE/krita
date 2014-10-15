/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_LIQUIFY_PROPERTIES_H
#define __KIS_LIQUIFY_PROPERTIES_H

#include <QtGlobal>


class KisLiquifyProperties {
public:
    enum LiquifyMode {
        MOVE,
        SCALE,
        ROTATE,
        OFFSET,
        UNDO
    };

    KisLiquifyProperties()
        : m_mode(MOVE),
          m_size(50),
          m_amount(0.2),
          m_spacing(0.2),
          m_sizeHasPressure(false),
          m_amountHasPressure(false),
          m_reverseDirection(false)
    {
    }

    LiquifyMode mode() const {
        return m_mode;
    }
    void setMode(LiquifyMode value) {
        m_mode = value;
    }

    qreal size() const {
        return m_size;
    }
    void setSize(qreal value) {
        m_size = value;
    }

    qreal amount() const {
        return m_amount;
    }
    void setAmount(qreal value) {
        m_amount = value;
    }

    qreal spacing() const {
        return m_spacing;
    }
    void setSpacing(qreal value) {
        m_spacing = value;
    }

    bool sizeHasPressure() const {
        return m_sizeHasPressure;
    }
    void setSizeHasPressure(bool value) {
        m_sizeHasPressure = value;
    }

    bool amountHasPressure() const {
        return m_amountHasPressure;
    }
    void setAmountHasPressure(bool value) {
        m_amountHasPressure = value;
    }

    bool reverseDirection() const {
        return m_reverseDirection;
    }
    void setReverseDirection(bool value) {
        m_reverseDirection = value;
    }

    void saveMode() const;
    void loadMode();

private:
    LiquifyMode m_mode;
    qreal m_size;
    qreal m_amount;
    qreal m_spacing;
    bool m_sizeHasPressure;
    bool m_amountHasPressure;
    bool m_reverseDirection;
};

#endif /* __KIS_LIQUIFY_PROPERTIES_H */
