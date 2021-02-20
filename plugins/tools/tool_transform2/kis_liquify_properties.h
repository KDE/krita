/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_LIQUIFY_PROPERTIES_H
#define __KIS_LIQUIFY_PROPERTIES_H

#include <QtGlobal>
#include <QDebug>
#include <boost/operators.hpp>
#include "kritatooltransform_export.h"

class QDomElement;


class KRITATOOLTRANSFORM_EXPORT KisLiquifyProperties : boost::equality_comparable<KisLiquifyProperties>
{
public:
    enum LiquifyMode {
        MOVE,
        SCALE,
        ROTATE,
        OFFSET,
        UNDO,

        N_MODES
    };

    KisLiquifyProperties()
        : m_mode(MOVE),
          m_size(60),
          m_amount(0.05),
          m_spacing(0.2),
          m_sizeHasPressure(false),
          m_amountHasPressure(false),
          m_reverseDirection(false),
          m_useWashMode(false),
          m_flow(0.2)
    {
    }


    KisLiquifyProperties(const KisLiquifyProperties &rhs);

    KisLiquifyProperties& operator=(const KisLiquifyProperties &rhs);

    bool operator==(const KisLiquifyProperties &other) const;

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

    static qreal minSize() {
        return 5.0;
    }

    static qreal maxSize() {
        return 1000.0;
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

    bool useWashMode() const {
        return m_useWashMode;
    }
    void setUseWashMode(bool value) {
        m_useWashMode = value;
    }

    qreal flow() const {
        return m_flow;
    }
    void setFlow(qreal value) {
        m_flow = value;
    }

    void saveMode() const;
    void loadMode();

    void loadAndResetMode();

    void toXML(QDomElement *e) const;
    static KisLiquifyProperties fromXML(const QDomElement &e);

private:
    LiquifyMode m_mode;
    qreal m_size;
    qreal m_amount;
    qreal m_spacing;
    bool m_sizeHasPressure;
    bool m_amountHasPressure;
    bool m_reverseDirection;

    bool m_useWashMode;
    qreal m_flow;
};

QDebug KRITATOOLTRANSFORM_EXPORT operator<<(QDebug dbg, const KisLiquifyProperties &properties);

#endif /* __KIS_LIQUIFY_PROPERTIES_H */
