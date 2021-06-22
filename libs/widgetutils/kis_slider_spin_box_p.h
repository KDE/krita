/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2010 Justin Noel <justin@ics.com>
 * SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2015 Moritz Molch <kde@moritzmolch.de>
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISSLIDERSPINBOXPRIVATE_H
#define KISSLIDERSPINBOXPRIVATE_H

#include <QObject>
#include <QLineEdit>
#include <QStyleOptionSpinBox>
#include <QPainter>
#include <QEvent>
#include <QApplication>
#include <QMouseEvent>
#include <QTimer>
#include <QStyleHints>
#include <QMenu>
#include <QVariantAnimation>
#include <QPointer>

#include <cmath>
#include <utility>
#include <type_traits>

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <kis_cursor.h>
#include <kis_num_parser.h>
#include <klocalizedstring.h>
#include <kis_painting_tweaks.h>
#include <kis_int_parse_spin_box.h>
#include <kis_double_parse_spin_box.h>
#include <kis_algebra_2d.h>
#include <kis_signal_compressor_with_param.h>

template <typename SpinBoxTypeTP, typename BaseSpinBoxTypeTP>
class Q_DECL_HIDDEN KisSliderSpinBoxPrivate : public QObject
{
public:
    using SpinBoxType = SpinBoxTypeTP;
    using BaseSpinBoxType = BaseSpinBoxTypeTP;
    using ValueType = decltype(std::declval<SpinBoxType>().value());

    enum ValueUpdateMode
    {
        ValueUpdateMode_NoChange,
        ValueUpdateMode_UseLastValidValue,
        ValueUpdateMode_UseValueBeforeEditing
    };

    KisSliderSpinBoxPrivate(SpinBoxType *q)
        : m_q(q)
        , m_lineEdit(m_q->lineEdit())
        , m_startEditingSignalProxy(std::bind(&KisSliderSpinBoxPrivate::startEditing, this))
    {
        m_q->installEventFilter(this);

        m_lineEdit->setReadOnly(true);
        m_lineEdit->setAlignment(Qt::AlignCenter);
        m_lineEdit->setAutoFillBackground(false);
        m_lineEdit->setCursor(KisCursor::splitHCursor());
        m_lineEdit->installEventFilter(this);

        m_widgetRangeToggle = new QWidget(m_q);
        m_widgetRangeToggle->hide();
        m_widgetRangeToggle->installEventFilter(this);

        m_timerStartEditing.setSingleShot(true);
        connect(&m_timerStartEditing, &QTimer::timeout, this, &KisSliderSpinBoxPrivate::startEditing);

        m_sliderAnimation.setStartValue(0.0);
        m_sliderAnimation.setEndValue(1.0);
        m_sliderAnimation.setEasingCurve(QEasingCurve(QEasingCurve::InOutCubic));
        connect(&m_sliderAnimation, &QVariantAnimation::valueChanged, m_lineEdit, QOverload<>::of(&QLineEdit::update));
        connect(&m_sliderAnimation, &QVariantAnimation::valueChanged, m_widgetRangeToggle, QOverload<>::of(&QLineEdit::update));

        m_rangeToggleHoverAnimation.setStartValue(0.0);
        m_rangeToggleHoverAnimation.setEndValue(1.0);
        m_rangeToggleHoverAnimation.setEasingCurve(QEasingCurve(QEasingCurve::InOutCubic));
        connect(&m_rangeToggleHoverAnimation, &QVariantAnimation::valueChanged, m_widgetRangeToggle, QOverload<>::of(&QLineEdit::update));
    }

    void startEditing()
    {
        if (isEditModeActive()) {
            return;
        }
        // Store the current value
        m_valueBeforeEditing = m_q->value();
        m_lineEdit->setReadOnly(false);
        m_q->selectAll();
        m_lineEdit->setFocus(Qt::OtherFocusReason);
        m_lineEdit->setCursor(KisCursor::ibeamCursor());
    }

    void endEditing(ValueUpdateMode updateMode = ValueUpdateMode_UseLastValidValue)
    {
        if (!isEditModeActive()) {
            return;
        }
        if (updateMode == ValueUpdateMode_UseLastValidValue) {
            setValue(m_q->value(), false, false, true);
        } else if (updateMode == ValueUpdateMode_UseValueBeforeEditing) {
            setValue(m_valueBeforeEditing, false, false, true);
        }
        // Restore palette colors
        QPalette pal = m_lineEdit->palette();
        pal.setBrush(QPalette::Text, m_q->palette().text());
        m_lineEdit->setPalette(pal);
        m_rightClickCounter = 0;
        m_lineEdit->setReadOnly(true);
        m_lineEdit->setCursor(KisCursor::splitHCursor());
        m_lineEdit->update();
        m_q->update();
    }
    
    bool isEditModeActive() const
    {
        return !m_lineEdit->isReadOnly();
    }

    // Compute a new value as a function of the x and y coordinates relative
    // to the lineedit, and some combination of modifiers
    ValueType valueForPoint(const QPoint &p, Qt::KeyboardModifiers modifiers) const
    {
        const QRectF rect(m_lineEdit->rect());
        const QPointF center(static_cast<double>(m_lastMousePressPosition.x()), rect.height() / 2.0);
        const bool useSoftRange = isSoftRangeValid() && (m_softRangeViewMode == SoftRangeViewMode_AlwaysShowSoftRange || m_isSoftRangeActive);
        const double minimum = static_cast<double>(useSoftRange ? m_softMinimum : m_q->minimum());
        const double maximum = static_cast<double>(useSoftRange ? m_softMaximum : m_q->maximum());
        const double rangeSize = maximum - minimum;
        // Get the distance relative to the line edit center and transformed
        // so that it starts counting 32px away from the widget. If the position
        // is inside the widget or that 32px area the distance will be 0 so that
        // the value change will be the same near the widget
        const double distanceY =
            std::max(
                0.0,
                std::abs(static_cast<double>(p.y()) - center.y()) - center.y() - constantDraggingMargin
            );
        // Get the scale
        double scale;
        if (modifiers & Qt::ShiftModifier) {
            // If the shift key is pressed we scale the distanceY value to make the scale
            // have a stronger effect and also offset it so that the minimum
            // scale will be 5x (1x + 4x).
            // function
            scale = (rect.width() + 2.0 * distanceY * 10.0) / rect.width() + 4.0;
        } else {
            // Get the scale as a function of the vertical position
            scale = (rect.width() + 2.0 * distanceY * 2.0) / rect.width();
        }
        // Scale the horizontal coordinates around where we first clicked 
        // as a function of the y coordinate
        const double scaledRectLeft = (0.0 - center.x()) * scale + center.x();
        const double scaledRectRight = (rect.width() - center.x()) * scale + center.x();
        // Map the current horizontal position to the new rect
        const double scaledRectWidth = scaledRectRight - scaledRectLeft;
        const double posX = static_cast<double>(p.x()) - scaledRectLeft;
        // Normalize
        const double normalizedPosX = posX / scaledRectWidth;
        // Final value
        const double normalizedValue = std::pow(normalizedPosX, m_exponentRatio);
        double value = normalizedValue * rangeSize + minimum;
        // If key CTRL is pressed, round to the closest step.
        if (modifiers & Qt::ControlModifier) {
            value = std::round(value / m_fastSliderStep) * m_fastSliderStep;
        }
        //Return the value
        if (std::is_same<ValueType, double>::value) {
            return qBound(minimum, value, maximum);
        } else {
            return static_cast<ValueType>(qBound(minimum, std::round(value), maximum));
        }
    }

    // Custom setValue that allows disabling signal emission
    void setValue(ValueType newValue,
                  bool blockSignals = false,
                  bool emitSignalsEvenWhenValueNotChanged = false,
                  bool overwriteExpression = false)
    {
        if (blockSignals) {
            m_q->blockSignals(true);
            m_q->BaseSpinBoxType::setValue(newValue, overwriteExpression);
            m_q->blockSignals(false);
        } else {
            ValueType v = m_q->value();
            m_q->BaseSpinBoxType::setValue(newValue, overwriteExpression);
            if (v == m_q->value() && emitSignalsEvenWhenValueNotChanged) {
                emitSignals();
            }
        }
        if (!m_q->hasFocus()) {
            endEditing(ValueUpdateMode_NoChange);
        }
    }

    void resetRangeMode()
    {
        if (isSoftRangeValid() && m_softRangeViewMode == SoftRangeViewMode_ShowBothRanges) {
            if (m_isSoftRangeActive) {
                makeSoftRangeActive();
            } else {
                makeHardRangeActive();
            }
            updateWidgetRangeToggleTooltip();
            m_widgetRangeToggle->show();
        } else {
            m_sliderAnimation.stop();
            m_widgetRangeToggle->hide();
        }
        qResizeEvent(nullptr);
    }

    template <typename U = SpinBoxTypeTP, typename = typename std::enable_if<std::is_same<ValueType, int>::value, U>::type>
    void setRange(int newMinimum, int newMaximum, bool computeNewFastSliderStep)
    {
        m_q->BaseSpinBoxType::setRange(newMinimum, newMaximum);
        if (computeNewFastSliderStep) {
            // Behavior taken from the old slider spinbox. Kind of arbitrary
            m_fastSliderStep = (m_q->maximum() - m_q->minimum()) / 20;
            if (m_fastSliderStep == 0) {
                m_fastSliderStep = 1;
            }
        }
        m_softMinimum = qBound(m_q->minimum(), m_softMinimum, m_q->maximum());
        m_softMaximum = qBound(m_q->minimum(), m_softMaximum, m_q->maximum());
        resetRangeMode();
        m_q->update();
    }

    template <typename U = SpinBoxTypeTP, typename = typename std::enable_if<std::is_same<ValueType, double>::value, U>::type>
    void setRange(double newMinimum, double newMaximum, int newNumberOfecimals, bool computeNewFastSliderStep)
    {
        m_q->setDecimals(newNumberOfecimals);
        m_q->BaseSpinBoxType::setRange(newMinimum, newMaximum);
        if (computeNewFastSliderStep) {
            // Behavior takem from the old slider. Kind of arbitrary
            const double rangeSize = m_q->maximum() - m_q->minimum();
            if (rangeSize >= 2.0 || newNumberOfecimals <= 0) {
                m_fastSliderStep = 1.0;
            } else if (newNumberOfecimals == 1) {
                m_fastSliderStep = rangeSize / 10.0;
            } else {
                m_fastSliderStep = rangeSize / 20.0;
            }
        }
        m_softMinimum = qBound(m_q->minimum(), m_softMinimum, m_q->maximum());
        m_softMaximum = qBound(m_q->minimum(), m_softMaximum, m_q->maximum());
        resetRangeMode();
        m_lineEdit->update();
    }

    void setBlockUpdateSignalOnDrag(bool newBlockUpdateSignalOnDrag)
    {
        m_blockUpdateSignalOnDrag = newBlockUpdateSignalOnDrag;
    }

    void setFastSliderStep(int newFastSliderStep)
    {
        m_fastSliderStep = newFastSliderStep;
    }

    // Set the soft range. Set newSoftMinimum = newSoftMaximum to signal that
    // the soft range must not be used
    void setSoftRange(ValueType newSoftMinimum, ValueType newSoftMaximum)
    {
        if ((newSoftMinimum != newSoftMaximum) &&
            (newSoftMinimum > newSoftMaximum || newSoftMinimum < m_q->minimum() || newSoftMaximum > m_q->maximum())) {
            return;
        }
        m_softMinimum = newSoftMinimum;
        m_softMaximum = newSoftMaximum;
        resetRangeMode();
        m_lineEdit->update();
    }

    bool isSoftRangeValid() const
    {
        return m_softMaximum > m_softMinimum;
    }

    ValueType fastSliderStep() const
    {
        return m_fastSliderStep;
    }

    ValueType softMinimum() const
    {
        return m_softMinimum;
    }

    ValueType softMaximum() const
    {
        return m_softMaximum;
    }

    bool isDragging() const
    {
        return m_isDragging;
    }
    
    void makeSoftRangeActive()
    {
        m_sliderAnimation.stop();
        m_isSoftRangeActive = true;
        // scale the animation duration in case the animation is in the middle
        const int animationDuration =
            static_cast<int>(std::round(m_sliderAnimation.currentValue().toReal() * fullAnimationDuration));
        m_sliderAnimation.setStartValue(m_sliderAnimation.currentValue());
        m_sliderAnimation.setEndValue(0.0);
        m_sliderAnimation.setDuration(animationDuration);
        m_sliderAnimation.start();
    }

    void makeHardRangeActive()
    {
        m_sliderAnimation.stop();
        m_isSoftRangeActive = false;
        // scale the duration in case the animation is in the middle
        const int animationDuration =
            static_cast<int>(std::round((1.0 - m_sliderAnimation.currentValue().toReal()) * fullAnimationDuration));
        m_sliderAnimation.setStartValue(m_sliderAnimation.currentValue());
        m_sliderAnimation.setEndValue(1.0);
        m_sliderAnimation.setDuration(animationDuration);
        m_sliderAnimation.start();
    }

    void setExponentRatio(double newExponentRatio)
    {
        m_exponentRatio = newExponentRatio;
        m_lineEdit->update();
    }

    void updateWidgetRangeToggleTooltip()
    {
        m_widgetRangeToggle->setToolTip(
            i18nc(
                "@info:tooltip toggle between soft and hard range in the slider spin box",
                QString("Toggle between full range and subrange.\nFull range: [%1, %2]\nSubrange: [%3, %4]")
                .arg(QString::number(m_q->minimum())).arg(QString::number(m_q->maximum()))
                .arg(QString::number(m_softMinimum)).arg(QString::number(m_softMaximum)).toUtf8()
            )
        );
    }

    QSize sizeHint() const
    {
        QSize hint = m_q->BaseSpinBoxType::sizeHint();
        return
            (isSoftRangeValid() && m_softRangeViewMode == SoftRangeViewMode_ShowBothRanges)
            ? QSize(hint.width() + widthOfRangeModeToggle, hint.height())
            : hint;
    }

    QSize minimumSizeHint() const
    {
        QSize hint = m_q->BaseSpinBoxType::minimumSizeHint();
        return
            (isSoftRangeValid() && m_softRangeViewMode == SoftRangeViewMode_ShowBothRanges)
            ? QSize(hint.width() + widthOfRangeModeToggle, hint.height())
            : hint;
    }

    void emitSignals() const
    {
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
        emit m_q->textChanged(m_q->text());
#else
        emit m_q->valueChanged(m_q->text());
#endif
        emit m_q->valueChanged(m_q->value());
    }

    bool qResizeEvent(QResizeEvent*)
    {
        // When resizing the spinbox, perform style specific positioning
        // of the lineedit
        
        // Get the default rect for the lineedit widget
        QStyleOptionSpinBox spinBoxOptions;
        m_q->initStyleOption(&spinBoxOptions);
        QRect rect = m_q->style()->subControlRect(QStyle::CC_SpinBox, &spinBoxOptions, QStyle::SC_SpinBoxEditField);
        // Offset the rect to make it take all the available space inside the
        // spinbox, without overlapping the buttons
        QString style = qApp->property(currentUnderlyingStyleNameProperty).toString().toLower();
        if (style == "breeze") {
            rect.adjust(-4, -4, 0, 4);
        } else if (style == "fusion") {
            rect.adjust(-2, -1, 2, 1);
        }
        // Set the rect
        if (isSoftRangeValid() && m_softRangeViewMode == SoftRangeViewMode_ShowBothRanges) {
            m_lineEdit->setGeometry(rect.adjusted(0, 0, -widthOfRangeModeToggle, 0));
            m_widgetRangeToggle->setGeometry(rect.adjusted(rect.width() - widthOfRangeModeToggle, 0, 0, 0));
        } else {
            m_lineEdit->setGeometry(rect);
        }

        return true;
    }

    bool qFocusOutEvent(QFocusEvent*)
    {
        // If the focus is lost then the edition stops, unless the focus
        // was lost because the menu was shown
        if (m_focusLostDueToMenu) {
            m_focusLostDueToMenu = false;
        } else {
            if (m_q->isLastValid()) {
                endEditing();
            }
        }
        return false;
    }

    bool qMousePressEvent(QMouseEvent*)
    {
        // If we click in any part of the spinbox outside the lineedit
        // then the edition stops
        endEditing();
        return false;
    }

    bool qKeyPressEvent(QKeyEvent *e)
    {
        switch (e->key()) {
            // If the lineedit is not in edition mode, make the left and right
            // keys have the same effect as the down and up keys. This replicates
            // the behaviour of the old slider spinbox
            case Qt::Key_Right:
                if (!isEditModeActive()) {
                    qApp->postEvent(m_q, new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, e->modifiers()));
                    return true;
                }
                break;
            case Qt::Key_Left:
                if (!isEditModeActive()) {
                    qApp->postEvent(m_q, new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, e->modifiers()));
                    return true;
                }
                break;
            // The enter key can be used to enter the edition mode if the
            // lineedit is not in it or to commit the entered value and leave
            // the edition mode if we are in it
            case Qt::Key_Enter:
            case Qt::Key_Return:
                if (!e->isAutoRepeat()) {
                    if (!isEditModeActive()) {
                        startEditing();
                    } else {
                        if (m_q->isLastValid()) {
                            endEditing();
                        } else {
                            return false;
                        }
                    }
                }
                return true;
            // The escape key can be used to leave the edition mode rejecting
            // the written value 
            case Qt::Key_Escape:
                if (isEditModeActive()) {
                    endEditing(ValueUpdateMode_UseValueBeforeEditing);
                    return true;
                }
                break;
            // If we press a number key when in slider mode, then start the edit
            // mode and resend the key event so that the key is processed again
            // in edit mode. Since entering edit mode selects all text, the new
            // event will replace the text by the new key text
            default:
                if (!isEditModeActive() && e->key() >= Qt::Key_0 && e->key() <= Qt::Key_9) {
                    startEditing();
                    qApp->postEvent(m_q, new QKeyEvent(QEvent::KeyPress, e->key(), e->modifiers(), e->text(), e->isAutoRepeat()));
                    return true;
                }
                break;
        }
        return false;
    }

    bool qContextMenuEvent(QContextMenuEvent *e)
    {
        // Shows a menu. Code inspired by the QAbstractSpinBox
        // contextMenuEvent function
        
        // Return if we are in slider mode and the mouse is in the line edit
        if (!isEditModeActive() && m_lineEdit->rect().contains(e->pos())) {
            return true;
        }
        // Create the menu
        QPointer<QMenu> menu;
        // If we are in edit mode, then add the line edit
        // actions
        if (isEditModeActive()) {
            menu = m_lineEdit->createStandardContextMenu();
            m_focusLostDueToMenu = true;
        } else {
            menu = new QMenu;
        }
        if (!menu) {
            return true;
        }
        // Override select all action
        QAction *selectAllAction = nullptr;
        if (isEditModeActive()) {
            selectAllAction = new QAction(i18nc("Menu action to select all text in the slider spin box", "&Select All"), menu);
#if QT_CONFIG(shortcut)
            selectAllAction->setShortcut(QKeySequence::SelectAll);
#endif
            menu->removeAction(menu->actions().last());
            menu->addAction(selectAllAction);
            menu->addSeparator();
        }
        // Add step up and step down actions
        const uint stepEnabled = m_q->stepEnabled();
        QAction *stepUpAction = menu->addAction(i18nc("Menu action to step up in the slider spin box", "&Step up"));
        stepUpAction->setEnabled(stepEnabled & SpinBoxType::StepUpEnabled);
        QAction *stepDown = menu->addAction(i18nc("Menu action to step down in the slider spin box", "Step &down"));
        stepDown->setEnabled(stepEnabled & SpinBoxType::StepDownEnabled);
        menu->addSeparator();
        // This code is taken from QAbstractSpinBox. Use a QPointer in case the
        // spin box is destroyed while the menu is shown??
        const QPointer<SpinBoxType> spinbox = m_q;
        const QPoint pos =
            (e->reason() == QContextMenuEvent::Mouse)
            ? e->globalPos()
            : m_q->mapToGlobal(QPoint(e->pos().x(), 0)) + QPoint(m_q->width() / 2, m_q->height() / 2);
        const QAction *action = menu->exec(pos);
        delete static_cast<QMenu *>(menu);
        if (spinbox && action) {
            if (action == stepUpAction) {
                m_q->stepBy(static_cast<ValueType>(1));
            } else if (action == stepDown) {
                m_q->stepBy(static_cast<ValueType>(-1));
            } else if (action == selectAllAction) {
                m_q->selectAll();
            }
        }
        e->accept();
        return true;
    }

    // Generic "style aware" helper function to draw a rect
    void paintSliderRect(QPainter &painter, const QRectF &rect, const QBrush &brush)
    {
        painter.save();
        painter.setBrush(brush);
        painter.setPen(Qt::NoPen);
        if (qApp->property(currentUnderlyingStyleNameProperty).toString().toLower() == "fusion") {
            painter.drawRoundedRect(rect, 1, 1);
        } else {
            painter.drawRoundedRect(rect, 0, 0);
        }
        painter.restore();
    }

    void paintSliderText(QPainter &painter, const QString &text, const QRectF &rect, const QRectF &clipRect, const QColor &color, const QTextOption &textOption)
    {
        painter.setBrush(Qt::NoBrush);
        painter.setPen(color);
        painter.setClipping(true);
        painter.setClipRect(clipRect);
        painter.drawText(rect, text, textOption);
        painter.setClipping(false);
    };

    void paintGenericSliderText(QPainter &painter, const QString &text, const QRectF &rect, const QRectF &sliderRect)
    {
        QTextOption textOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter);
        textOption.setWrapMode(QTextOption::NoWrap);
        // Draw portion of the text that is over the background
        paintSliderText(painter, text, rect, rect.adjusted(sliderRect.width(), 0, 0, 0), m_lineEdit->palette().text().color(), textOption);
        // Draw portion of the text that is over the progress bar
        paintSliderText(painter, text, rect, sliderRect, m_lineEdit->palette().highlightedText().color(), textOption);
    };

    void paintSlider(QPainter &painter, const QString &text, double slider01Width, double slider02Width = -1.0)
    {
        const QRectF rect = m_lineEdit->rect();
        const QColor highlightColor = m_q->palette().highlight().color();
        if (slider02Width < 0.0) {
            const QRectF sliderRect = rect.adjusted(0, 0, -(rect.width() - slider01Width), 0);
            paintSliderRect(painter, sliderRect, highlightColor);
            if (!text.isNull()) {
                paintGenericSliderText(painter, text, rect, sliderRect);
            }
        } else {
            static constexpr double heightOfCollapsedSliderPlusSpace = heightOfCollapsedSlider + heightOfSpaceBetweenSliders;
            const double heightBetween = rect.height() - 2.0 * heightOfCollapsedSlider - heightOfSpaceBetweenSliders;
            const double animationPos = m_sliderAnimation.currentValue().toReal();
            const double a = heightOfCollapsedSliderPlusSpace;
            const double b = heightOfCollapsedSliderPlusSpace + heightBetween;
            // Paint background text
            QTextOption textOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter);
            textOption.setWrapMode(QTextOption::NoWrap);
            paintSliderText(painter, text, rect, rect, m_lineEdit->palette().text().color(), textOption);
            // Paint soft range slider
            const QColor softSliderColor = KisPaintingTweaks::blendColors(highlightColor, m_q->palette().base().color(), 1.0 - 0.25);
            const double softSliderAdjustment = -KisAlgebra2D::lerp(a, b, animationPos);
            paintSliderRect(
                painter,
                rect.adjusted(0, 0, -(rect.width() - slider02Width), softSliderAdjustment),
                softSliderColor
            );
            // Paint hard range slider
            const double hardSliderAdjustment = KisAlgebra2D::lerp(a, b, 1.0 - animationPos);
            paintSliderRect(
                painter,
                rect.adjusted(0, hardSliderAdjustment, -(rect.width() - slider01Width), 0),
                highlightColor
            );
            // Paint text in the sliders
            paintSliderText(
                painter, text, rect,
                rect.adjusted(0, 0, -(rect.width() - slider02Width), softSliderAdjustment),
                m_lineEdit->palette().highlightedText().color(),
                textOption
            );
            paintSliderText(
                painter, text, rect,
                rect.adjusted(0, hardSliderAdjustment, -(rect.width() - slider01Width), 0),
                m_lineEdit->palette().highlightedText().color(),
                textOption
            );
        }
    }

    double computeSliderWidth(double min, double max, double value) const
    {
        const double rangeSize = max - min;
        const double localPosition = value - min;
        const double normalizedValue = std::pow(localPosition / rangeSize, 1.0 / m_exponentRatio);
        const double width = static_cast<double>(m_lineEdit->width());
        return qBound(0.0, std::round(normalizedValue * width), width);
    }

    bool lineEditPaintEvent(QPaintEvent*)
    {
        QPainter painter(m_lineEdit);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const double value = m_q->value();
        
        // If we are not editing, just draw the text, otherwise draw a 
        // semi-transparent rect to dim the background and let the QLineEdit
        // draw the rest (text, selection, cursor, etc.)
        const double hardSliderWidth = computeSliderWidth(static_cast<double>(m_q->minimum()), static_cast<double>(m_q->maximum()), value);
        const double softSliderWidth = computeSliderWidth(m_softMinimum, m_softMaximum, value);
        if (!isEditModeActive()) {
            QString text = m_q->text();
            if (isSoftRangeValid()) {
                if (m_softRangeViewMode == SoftRangeViewMode_AlwaysShowSoftRange) {
                    paintSlider(painter, text, softSliderWidth);
                } else {
                    paintSlider(painter, text, hardSliderWidth, softSliderWidth);
                }
            } else {
                // Draw the slider
                paintSlider(painter, text, hardSliderWidth);
            }
        } else {
            // Draw the slider
            if (isSoftRangeValid()) {
                if (m_softRangeViewMode == SoftRangeViewMode_AlwaysShowSoftRange) {
                    paintSlider(painter, QString(), softSliderWidth);
                } else {
                    paintSlider(painter, QString(), hardSliderWidth, softSliderWidth);
                }
            } else {
                paintSlider(painter, QString(), hardSliderWidth);
            }
            // Paint the overlay with the base color
            QColor color = m_q->palette().base().color();
            color.setAlpha(128);
            paintSliderRect(painter, m_lineEdit->rect(), color);
        }
        // If we are editing the text then return false and let the QLineEdit
        // paint all the edit related stuff (e.g. selection)
        return !isEditModeActive();
    }

    bool lineEditMousePressEvent(QMouseEvent *e)
    {
        if (!m_q->isEnabled()) {
            return false;
        }
        if (!isEditModeActive()) {
            // Pressing and holding the left button in the lineedit in slider
            // mode starts a timer which makes the lineedit enter
            // edition mode if it is completed
            if (e->button() == Qt::LeftButton) {
                m_lastMousePressPosition = e->pos();
                m_timerStartEditing.start(qApp->styleHints()->mousePressAndHoldInterval());
            }
            return true;
        }
        return false;
    }

    bool lineEditMouseReleaseEvent(QMouseEvent *e)
    {
        if (!m_q->isEnabled()) {
            return false;
        }
        if (!isEditModeActive()) {
            // Releasing the right mouse button makes the lineedit enter
            // the edition mode if we are not editing
            if (e->button() == Qt::RightButton) {
                // If we call startEditing() right from the eventFilter(),
                // then the mouse release event will be somehow be passed
                // to Qt further and generate ContextEvent on Windows.
                // Therefore we should call it from a normal timer event.
                QTimer::singleShot(0, &m_startEditingSignalProxy, SLOT(start()));
            // Releasing the left mouse button stops the dragging and also
            // the "enter edition mode" timer. If signals must be blocked when
            // dragging then we set the value here and emit a signal
            } else if (e->button() == Qt::LeftButton) {
                m_timerStartEditing.stop();
                m_isDragging = false;
                setValue(valueForPoint(e->pos(), e->modifiers()), false, true);
            }
            return true;
        }
        return false;
    }

    bool lineEditMouseMoveEvent(QMouseEvent *e)
    {
        if (!m_q->isEnabled()) {
            return false;
        }
        if (!isEditModeActive()) {
            if (e->buttons() & Qt::LeftButton) {
                // If the timer is active that means we pressed the button in
                // slider mode
                if (m_timerStartEditing.isActive()) {
                    const int dx = e->pos().x() - m_lastMousePressPosition.x();
                    const int dy = e->pos().y() - m_lastMousePressPosition.y();
                    // If the mouse position is still close to the point where
                    // we pressed, then we still wait for the "enter edit mode"
                    // timer to complete
                    if (dx * dx + dy * dy <= startDragDistanceSquared) {
                        return true;
                    // If the mouse moved far from where we first pressed, then
                    // stop the timer and start dragging
                    } else {
                        m_timerStartEditing.stop();
                        m_isDragging = true;
                    }
                }
                // At this point we are dragging so record the position and set
                // the value
                m_lastMousePosition = e->pos();
                setValue(valueForPoint(e->pos(), e->modifiers()), m_blockUpdateSignalOnDrag);
                return true;
            }
        }
        return false;
    }

    bool widgetRangeTogglePaintEvent(QPaintEvent*)
    {
        QPainter painter(m_widgetRangeToggle);
        painter.setRenderHint(QPainter::Antialiasing, true);
        // Compute sizes and positions
        const double width = static_cast<double>(m_widgetRangeToggle->width());
        const double height = static_cast<double>(m_widgetRangeToggle->height());
        constexpr double marginX = 4.0;
        const double toggleWidth = width - 2.0 * marginX;
        const double centerX = width * 0.5;
        const double centerY = height * 0.5;
        const double bigRadius = centerX - std::floor(centerX - (toggleWidth * 0.5)) + 0.5;
        const double smallRadius = bigRadius * 0.5;
        const double sliderAnimationPos = m_sliderAnimation.currentValue().toReal();
        const double radius = smallRadius + sliderAnimationPos * (bigRadius - smallRadius);
        // Compute color
        const double rangeToggleHoverAnimationPos = m_rangeToggleHoverAnimation.currentValue().toReal();
        const QColor baseColor = m_q->palette().base().color();
        const QColor textColor = m_q->palette().text().color();
        const QColor color = KisPaintingTweaks::blendColors(baseColor, textColor, 1.0 - (0.60 + 0.40 * rangeToggleHoverAnimationPos));
        // Paint outer circle
        painter.setPen(color);
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(QPointF(centerX, centerY), bigRadius, bigRadius);
        // Paint dot
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawEllipse(QPointF(centerX, centerY), radius, radius);
        return true;
    }

    bool widgetRangeToggletMouseReleaseEvent(QMouseEvent *e)
    {
        if (!m_q->isEnabled()) {
            return false;
        }
        if (e->button() == Qt::LeftButton) {
            if (!m_isSoftRangeActive) {
                makeSoftRangeActive();
            } else {
                makeHardRangeActive();
            }
            return true;
        }
        return false;
    }

    bool widgetRangeToggleEnterEvent(QEvent*)
    {
        m_rangeToggleHoverAnimation.stop();
        // scale the animation duration in case the animation is in the middle
        const int animationDuration =
            static_cast<int>(std::round(m_rangeToggleHoverAnimation.currentValue().toReal() * fullAnimationDuration));
        m_rangeToggleHoverAnimation.setStartValue(m_rangeToggleHoverAnimation.currentValue());
        m_rangeToggleHoverAnimation.setEndValue(1.0);
        m_rangeToggleHoverAnimation.setDuration(animationDuration);
        m_rangeToggleHoverAnimation.start();
        return false;
    }

    bool widgetRangeToggleLeaveEvent(QEvent*)
    {
        m_rangeToggleHoverAnimation.stop();
        // scale the animation duration in case the animation is in the middle
        const int animationDuration =
            static_cast<int>(std::round(m_rangeToggleHoverAnimation.currentValue().toReal() * fullAnimationDuration));
        m_rangeToggleHoverAnimation.setStartValue(m_rangeToggleHoverAnimation.currentValue());
        m_rangeToggleHoverAnimation.setEndValue(0.0);
        m_rangeToggleHoverAnimation.setDuration(animationDuration);
        m_rangeToggleHoverAnimation.start();
        return false;
    }

    bool eventFilter(QObject * o, QEvent * e) override
    {
        if (!o || !e) {
            return false;
        }
        if (o == m_q) {
            switch (e->type()) {
                case QEvent::Resize : return qResizeEvent(static_cast<QResizeEvent*>(e));
                case QEvent::FocusOut : return qFocusOutEvent(static_cast<QFocusEvent*>(e));
                case QEvent::MouseButtonPress : return qMousePressEvent(static_cast<QMouseEvent*>(e));
                case QEvent::KeyPress : return qKeyPressEvent(static_cast<QKeyEvent*>(e));
                case QEvent::ContextMenu : return qContextMenuEvent(static_cast<QContextMenuEvent*>(e));
                default: break;
            }
        } else if (o == m_lineEdit) {
            switch (e->type()) {
                case QEvent::Paint : return lineEditPaintEvent(static_cast<QPaintEvent*>(e));
                case QEvent::MouseButtonPress : return lineEditMousePressEvent(static_cast<QMouseEvent*>(e));
                case QEvent::MouseButtonRelease : return lineEditMouseReleaseEvent(static_cast<QMouseEvent*>(e));
                case QEvent::MouseMove : return lineEditMouseMoveEvent(static_cast<QMouseEvent*>(e));
                default: break;
            }
        } else if (o == m_widgetRangeToggle) {
            switch (e->type()) {
                case QEvent::Paint : return widgetRangeTogglePaintEvent(static_cast<QPaintEvent*>(e));
                case QEvent::MouseButtonRelease: return widgetRangeToggletMouseReleaseEvent(static_cast<QMouseEvent*>(e));
                case QEvent::Enter: return widgetRangeToggleEnterEvent(e);
                case QEvent::Leave: return widgetRangeToggleLeaveEvent(e);
                default: break;
            }
        }
        return false;
    }

private:
    // Distance that the pointer must move to start dragging
    static constexpr int startDragDistance{2};
    static constexpr int startDragDistanceSquared{startDragDistance * startDragDistance};
    // Margin around the spinbox for which the dragging gives same results,
    // regardless of the vertical distance
    static constexpr double constantDraggingMargin{32.0};
    // Height of the collapsed slider bar
    static constexpr double heightOfCollapsedSlider{3.0};
    // Height of the space between the soft and hard range sliders
    static constexpr double heightOfSpaceBetweenSliders{0.0};
    // Width of the area to activate soft/hard range
    static constexpr double widthOfRangeModeToggle{16.0};
    // The duration of the animation 
    static constexpr double fullAnimationDuration{200.0};

    SpinBoxType *m_q{nullptr};
    QLineEdit *m_lineEdit{nullptr};
    QWidget *m_widgetRangeToggle{nullptr};
    QTimer m_timerStartEditing;
    ValueType m_softMinimum{static_cast<ValueType>(0)};
    ValueType m_softMaximum{static_cast<ValueType>(0)};
    double m_exponentRatio{1.0};
    bool m_blockUpdateSignalOnDrag{false};
    ValueType m_fastSliderStep{static_cast<ValueType>(5)};
    mutable ValueType m_valueBeforeEditing;
    bool m_isDragging{false};
    QPoint m_lastMousePressPosition;
    QPoint m_lastMousePosition;
    int m_rightClickCounter{0};
    bool m_focusLostDueToMenu{false};
    bool m_isSoftRangeActive{true};
    QVariantAnimation m_sliderAnimation;
    QVariantAnimation m_rangeToggleHoverAnimation;
    SignalToFunctionProxy m_startEditingSignalProxy;

    enum SoftRangeViewMode
    {
        SoftRangeViewMode_AlwaysShowSoftRange,
        SoftRangeViewMode_ShowBothRanges
    } m_softRangeViewMode{SoftRangeViewMode_ShowBothRanges};
};

#endif // KISSLIDERSPINBOXPRIVATE_H
