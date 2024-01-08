/*
 * SPDX-FileCopyrightText: 2010 Justin Noel <justin@ics.com>
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef LIBKIS_SLIDERSPINBOX_H
#define LIBKIS_SLIDERSPINBOX_H

#include "kis_slider_spin_box.h"
#include "IntParseSpinBox.h"
#include "DoubleParseSpinBox.h"

#include "kritalibkis_export.h"
#include "libkis.h"

template <typename SpinBoxTypeTP, typename BaseSpinBoxTypeTP>
class KisSliderSpinBoxPrivate;

/**
 * @brief This class is a wrapper around KisSliderSpinBox, a spinbox in which 
 * you can click and drag to set the value, with a slider like bar displayed
 * inside. The widget itself is accessed with the widget() function.
 * 
 * The value can be set by click and dragging with the mouse or pen or by
 * typing in with the keyboard. To enter the edit mode, in which the keyboard
 * can be used, one has to right-click inside the spinbox or click and hold
 * the pointer inside or press the enter key. To leave the edit mode, one
 * can press the enter key again, in which case the value is committed, or
 * press the escape key, in which case the value is rejected.
 * 
 * When dragging with the pointer, one can fine tune the value by dragging
 * far away vertically from the spinbox. The farther the pointer is, the
 * slower the value will change. If the pointer is inside the spinbox plus
 * a certain margin, the value will not be scaled.
 * By pressing the shift key the slow down will be even more pronounced and
 * by pressing the control key the value will snap to the increment set by
 * @ref setFastSliderStep. The two keys can be used at the same time.
 * 
 * A "soft range" can be set to make the slider display only a sub-range of the
 * spinbox range. This way one can have a large range but display and set with
 * the pointer and with more precision only the most commonly used sub-set
 * of values.
 * A value outside the "soft range" can be set by entering the edit
 * mode and using the keyboard.
 * The "soft range" is considered valid if the "soft maximum" is greater than
 * the "soft minimum".
 */
class KRITALIBKIS_EXPORT SliderSpinBox : public IntParseSpinBox
{
    Q_OBJECT
    Q_DISABLE_COPY(SliderSpinBox)


public:
    explicit SliderSpinBox();
    ~SliderSpinBox() override;

public Q_SLOTS:

    /**
     * @brief Get the internal KisSliderSpinBox as a QWidget, so it may be
     * added to a UI
     * 
     * @return the internal KisSliderSpinBox as a QWidget
     */
    QWidget* widget() const;

    /**
     * @brief Get the value to which multiples the spinbox value snaps when
     * the control key is pressed
     * 
     * @return the value to which multiples the spinbox value snaps when
     * the control key is pressed
     * @see setFastSliderStep(int)
     */
    int fastSliderStep() const;
    /**
     * @brief Get the minimum value of the "soft range"
     * @return the minimum value of the "soft range"
     * @see setSoftMinimum(int) 
     * @see setSoftRange(int, int) 
     * @see softMaximum() const 
     */
    int softMinimum() const;
    /**
     * @brief Get the maximum value of the "soft range"
     * @return the maximum value of the "soft range"
     * @see setSoftMaximum(int) 
     * @see setSoftRange(int, int) 
     * @see softMinimum) const 
     */
    int softMaximum() const;
    /**
     * @brief Get if the user is currently dragging the slider with the pointer
     * @return true if the user is currently dragging the slider with the
     * pointer, false otherwise
     */
    bool isDragging() const;
    /**
     * @brief Set the value
     * @param newValue the new value
     */
    void setValue(int newValue);
    /**
     * @brief Set the minimum and the maximum values of the range, computing
     * a new "fast slider step" based on the range if required
     * 
     * The soft range will be adapted to fit inside the range
     * @param newMinimum the new minimum value
     * @param newMaximum the new maximum value
     * @param computeNewFastSliderStep true if a new "fast slider step"
     * must be computed based on the range
     * @see setMinimum(int)
     * @see setMaximum(int)
     */
    void setRange(int newMinimum, int newMaximum, bool computeNewFastSliderStep = true);
    /**
     * @brief Set the minimum value of the range
     * 
     * The soft range will be adapted to fit inside the range
     * @param newMinimum the new minimum value
     * @param computeNewFastSliderStep true if a new "fast slider step"
     * must be computed based on the range
     * @see setRange(int,int)
     * @see setMaximum(int)
     */
    void setMinimum(int newMinimum, bool computeNewFastSliderStep = true);
    /**
     * @brief Set the maximum value of the range
     * 
     * The soft range will be adapted to fit inside the range
     * @param newMaximum the new maximum value
     * @param computeNewFastSliderStep true if a new "fast slider step"
     * must be computed based on the range
     * @see setRange(int,int)
     * @see setMinimum(int)
     */
    void setMaximum(int newMaximum, bool computeNewFastSliderStep = true);
    /**
     * @brief Set the exponent used by a power function to modify the values
     * as a function of the horizontal position.
     * 
     * This allows having more values concentrated in one side of the
     * slider than the other
     * @param newExponentRatio the new exponent to be used by the power function
     */
    void setExponentRatio(qreal newExponentRatio);
    /**
     * @brief Set if the spinbox should not emit signals when dragging the
     * slider.
     * 
     * This is useful to prevent multiple updates when changing the value if
     * the update operation is costly.
     * A valueChanged signal will be emitted when the pointer is released from
     * the slider.
     * @param newBlockUpdateSignalOnDrag true if the spinbox should not emit
     * signals when dragging the slider. false otherwise
     */
    void setBlockUpdateSignalOnDrag(bool newBlockUpdateSignalOnDrag);
    /**
     * @brief Set the value to which multiples the spinbox value snaps when
     * the control key is pressed
     * @param newFastSliderStep value to which multiples the spinbox value
     * snaps when the control key is pressed
     * @see fastSliderStep() const
     */
    void setFastSliderStep(int newFastSliderStep);
    /**
     * @brief Set the minimum and the maximum values of the soft range
     * @param newSoftMinimum the new minimum value
     * @param newSoftMaximum the new maximum value
     * @see setSoftMinimum(int)
     * @see setSoftMaximum(int)
     * @see softMinimum() const
     * @see softMaximum() const
     */
    void setSoftRange(int newSoftMinimum, int newSoftMaximum);
    /**
     * @brief Set the minimum value of the soft range
     * @param newSoftMinimum the new minimum value
     * @see setSoftRange(int,int)
     * @see setSoftMaximum(int)
     * @see softMinimum() const
     * @see softMaximum() const
     */
    void setSoftMinimum(int newSoftMinimum);
    /**
     * @brief Set the maximum value of the soft range
     * @param newSoftMaximum the new maximum value
     * @see setSoftRange(int,int)
     * @see setSoftMinimum(int)
     * @see softMinimum() const
     * @see softMaximum() const
     */
    void setSoftMaximum(int newSoftMaximum);

Q_SIGNALS:
    void draggingFinished();

private:
    struct Private;
    Private *const d;
};

/**
 * @brief This class is a wrapper around KisDoubleSliderSpinBox, a spinbox in
 * which you can click and drag to set the value, with a slider like bar
 * displayed inside. The widget itself is accessed with the widget() function.
 * 
 * @see SliderSpinBox
 */
class KRITALIBKIS_EXPORT DoubleSliderSpinBox : public DoubleParseSpinBox
{
    Q_OBJECT
    Q_DISABLE_COPY(DoubleSliderSpinBox)

public:
    explicit DoubleSliderSpinBox();
    ~DoubleSliderSpinBox() override;

public Q_SLOTS:

    /**
     * @brief Get the internal KisDoubleSliderSpinBox as a QWidget, so it may be
     * added to a UI
     * 
     * @return the internal KisDoubleSliderSpinBox as a QWidget
     */
    QWidget* widget() const;

    qreal fastSliderStep() const;
    qreal softMinimum() const;
    qreal softMaximum() const;
    bool isDragging() const;
    void setValue(qreal newValue);
    /**
     * @brief Set the minimum and the maximum values of the range
     * 
     * The soft range will be adapted to fit inside the range
     * The number of decimals used can be changed with the newNumberOfDecimals
     * parameter
     * @param newMinimum the new minimum value
     * @param newMaximum the new maximum value
     * @param newNumberOfDecimals the new number of decimals
     * @param computeNewFastSliderStep true if a new "fast slider step"
     * must be computed based on the range
     * @see setMinimum(qreal)
     * @see setMaximum(qreal)
     */
    void setRange(qreal newMinimum, qreal newMaximum, int newNumberOfDecimals = 0, bool computeNewFastSliderStep = true);
    void setMinimum(qreal newMinimum, bool computeNewFastSliderStep = true);
    void setMaximum(qreal newMaximum, bool computeNewFastSliderStep = true);
    void setExponentRatio(qreal newExponentRatio);
    void setBlockUpdateSignalOnDrag(bool newBlockUpdateSignalOnDrag);
    void setFastSliderStep(qreal newFastSliderStep);
    void setSoftRange(qreal newSoftMinimum, qreal newSoftMaximum);
    void setSoftMinimum(qreal newSoftMinimum);
    void setSoftMaximum(qreal newSoftMaximum);

Q_SIGNALS:
    void draggingFinished();

private:
    struct Private;
    Private *const d;
};

#endif // LIBKIS_SLIDERSPINBOX_H
