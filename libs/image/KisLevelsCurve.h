/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_LEVELS_CURVE_H
#define KIS_LEVELS_CURVE_H

#include <QtGlobal>
#include <QVector>

#include "kritaimage_export.h"

/**
 * @brief This class holds the parameters for a levels adjustment. It is modeled
 * after KisCubicCurve and has similar interface and functionality
 */
class KRITAIMAGE_EXPORT KisLevelsCurve
{
public:
    /**
     * @brief Default value for the input black point
     */
    static constexpr qreal defaultInputBlackPoint() { return 0.0; }
    /**
     * @brief Default value for the input white point
     */
    static constexpr qreal defaultInputWhitePoint() { return 1.0; }
    /**
     * @brief Default value for the input gamma
     */
    static constexpr qreal defaultInputGamma() { return 1.0; }
    /**
     * @brief Default value for the output black point
     */
    static constexpr qreal defaultOutputBlackPoint() { return 0.0; }
    /**
     * @brief Default value for the output white point
     */
    static constexpr qreal defaultOutputWhitePoint() { return 1.0; }

    KisLevelsCurve();
    KisLevelsCurve(qreal inputBlackPoint, qreal inputWhitePoint, qreal inputGamma,
                   qreal outputBlackPoint, qreal outputWhitePoint);
    KisLevelsCurve(const KisLevelsCurve &rhs) = default;
    KisLevelsCurve& operator=(const KisLevelsCurve &rhs) = default;
    bool operator==(const KisLevelsCurve& rhs) const;

    /**
     * @brief Evaluates the function formed by the levels parameters for a
     * given x. The input and output values are normalized
     */
    qreal value(qreal x) const;
    
    /**
     * @brief Get the input black point
     */
    qreal inputBlackPoint() const;
    /**
     * @brief Get the input white point
     */
    qreal inputWhitePoint() const;
    /**
     * @brief Get the gamma value
     */
    qreal inputGamma() const;
    /**
     * @brief Get the output black point
     */
    qreal outputBlackPoint() const;
    /**
     * @brief Get the output white point
     */
    qreal outputWhitePoint() const;

    /**
     * @brief Set the input black point
     */
    void setInputBlackPoint(qreal newInputBlackPoint);
    /**
     * @brief Set the input white point
     */
    void setInputWhitePoint(qreal newInputWhitePoint);
    /**
     * @brief Set the gamma value
     */
    void setInputGamma(qreal newInputGamma);
    /**
     * @brief Set the output black point
     */
    void setOutputBlackPoint(qreal newOutputBlackPoint);
    /**
     * @brief Set the output white point
     */
    void setOutputWhitePoint(qreal newOutputWhitePoint);

    /**
     * @brief Resets the input and output levels (and gamma)
     */
    void resetAll();
    /**
     * @brief Resets the input levels only (and gamma)
     */
    void resetInputLevels();
    /**
     * @brief Resets the output levels only
     */
    void resetOutputLevels();

    /**
     * @brief Check whether the level info maps all values to themselves.
     */
    bool isIdentity() const;

    /**
     * @brief Get the name associated with this levels info object
     */
    const QString& name() const;
    /**
     * @brief Set the name associated with this levels info object. This allows
     * us to carry around a display name for the level info internally. It could
     * potentially be useful anywhere level info are used in the UI
     */
    void setName(const QString &newName);

    /**
     * @brief Returns a vector of size @param size with values obtained by
     * evaluating the function formed by the levels parameters from 0.0 to 1.0.
     * The resulting values are scaled to the range [0, 0xFF]
     * @param size The size of the returned vector
     * @return const QVector<quint16>& The vector with the values
     * @see value()
     * @see floatTransfer()
     */
    const QVector<quint16>& uint16Transfer(int size = 256) const;
    /**
     * @brief Returns a vector of size @param size with values obtained by
     * evaluating the function formed by the levels parameters from 0.0 to 1.0.
     * The resulting values are in the range [0, 1]
     * @param size The size of the returned vector
     * @return const QVector<qreal>& The vector with the values
     * @see value()
     * @see uint16Transfer()
     */
    const QVector<qreal>& floatTransfer(int size = 256) const;

    /**
     * @brief Get a text representation of the parameters. The format is:
     * "input_black_point;input_white_point;input_gamma;output_black_point;output_white_point".
     * For example: "0;1;0.6;0;1", "0.2;0.8;1.2;0.25;0.75"
     * @see fromString()
     */
    QString toString() const;
    /**
     * @brief Parses the parameters from a given text
     * @see toString
     */
    void fromString(const QString &text, bool *ok = nullptr);

private:
    qreal m_inputBlackPoint, m_inputWhitePoint, m_inputGamma;
    qreal m_outputBlackPoint, m_outputWhitePoint;
    qreal m_inputLevelsDelta, m_inverseInputGamma, m_outputLevelsDelta;
    QString m_name;
    mutable QVector<quint16> m_u16Transfer;
    mutable QVector<qreal> m_fTransfer;
    mutable bool m_mustRecomputeU16Transfer;
    mutable bool m_mustRecomputeFTransfer;

    void invalidate();
};

#endif
