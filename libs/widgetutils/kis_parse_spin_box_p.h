/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2016 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISPARSESPINBOXPRIVATE_H
#define KISPARSESPINBOXPRIVATE_H

#include <QTimer>
#include <QVariantAnimation>
#include <QValidator>
#include <QLineEdit>
#include <QIcon>
#include <QFile>
#include <QPainter>
#include <QApplication>
#include <QStyleOptionSpinBox>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QResizeEvent>

#include <cmath>
#include <utility>
#include <type_traits>

#include <kis_painting_tweaks.h>
#include <kis_num_parser.h>
#include <kis_algebra_2d.h>

template <typename SpinBoxTypeTP, typename BaseSpinBoxTypeTP>
class Q_DECL_HIDDEN KisParseSpinBoxPrivate : public QObject
{
public:
    using SpinBoxType = SpinBoxTypeTP;
    using BaseSpinBoxType = BaseSpinBoxTypeTP;
    using ValueType = decltype(std::declval<SpinBoxType>().value());
    
    KisParseSpinBoxPrivate(SpinBoxType *q)
        : m_q(q)
        , m_lineEdit(m_q->lineEdit())
    {
        m_q->setAlignment(Qt::AlignRight);
        m_q->installEventFilter(this);

        m_lineEdit->setAutoFillBackground(false);
        m_lineEdit->installEventFilter(this);
        connect(m_lineEdit, &QLineEdit::selectionChanged, this, &KisParseSpinBoxPrivate::fixupSelection);
        connect(m_lineEdit, &QLineEdit::cursorPositionChanged, this, &KisParseSpinBoxPrivate::fixupCursorPosition);

        m_timerShowWarning.setSingleShot(true);
        connect(&m_timerShowWarning, &QTimer::timeout, this, QOverload<>::of(&KisParseSpinBoxPrivate::showWarning));
        if (m_warningIcon.isNull() && QFile(":/./16_light_warning.svg").exists()) {
            m_warningIcon = QIcon(":/./16_light_warning.svg");
        }
        m_warningAnimation.setStartValue(0.0);
        m_warningAnimation.setEndValue(1.0);
        m_warningAnimation.setEasingCurve(QEasingCurve(QEasingCurve::InOutCubic));
        connect(&m_warningAnimation, &QVariantAnimation::valueChanged, m_lineEdit, QOverload<>::of(&QLineEdit::update));
    }
    
    void stepBy(int steps)
    {
        if (steps == 0) {
            return;
        }
        // Use the reimplementation os setValue in this function so that we can
        // clear the expression
        setValue(m_q->value() + static_cast<ValueType>(steps) * m_q->singleStep(), true);
        m_q->selectAll();
    }

    void setValue(ValueType value, bool overwriteExpression = false)
    {
        // The expression should be always cleared if the user is not
        // actively editing the text
        if (!m_q->hasFocus() || m_lineEdit->isReadOnly()) {
            overwriteExpression = true;
        }
        // Clear the expression so that the line edit shows just the
        // current value with prefix and suffix
        if (overwriteExpression) {
            m_lastExpressionParsed = QString();
        }
        // Prevent setting the new value if it is equal to the current one.
        // That will maintain the current expression and warning status.
        // If the value is different or the expression should overwritten then
        // the value is set and the warning status is cleared
        if (value != m_q->value() || overwriteExpression) {
            m_q->BaseSpinBoxType::setValue(value);
            if (!m_isLastValid) {
                m_isLastValid = true;
                hideWarning();
                emit m_q->noMoreParsingError();
            }
        }
    }

    bool isLastValid() const
    {
        return m_isLastValid;
    }

    QString veryCleanText() const
    {
        return m_q->cleanText();
    }

    QValidator::State validate(QString&, int&) const
    {
        // We want the user to be able to write any kind of expression.
        // If it produces a valid value or not is decided in "valueFromText"
        return QValidator::Acceptable;
    }

    // Helper function to evaluate a math expression string into an int
    template <typename U = SpinBoxTypeTP, typename = typename std::enable_if<std::is_same<ValueType, int>::value, U>::type>
    int parseMathExpression(const QString &text, bool *ok) const
    {
        return KisNumericParser::parseIntegerMathExpr(text, ok);
    }

    // Helper function to evaluate a math expression string into a double
    template <typename U = SpinBoxTypeTP, typename = typename std::enable_if<std::is_same<ValueType, double>::value, U>::type>
    double parseMathExpression(const QString &text, bool *ok) const
    {
        double value = KisNumericParser::parseSimpleMathExpr(text, ok);
        if(qIsNaN(value) || qIsInf(value)){
            *ok = false;
        }
        return value;
    }

    ValueType valueFromText(const QString &text) const
    {
        // Always hide the warning when the text changes
        hideWarning();
        // Get the expression, removing the prefix and suffix
        m_lastExpressionParsed = text;
        if (m_lastExpressionParsed.endsWith(m_q->suffix())) {
            m_lastExpressionParsed.remove(m_lastExpressionParsed.size() - m_q->suffix().size(), m_q->suffix().size());
        }
        if(m_lastExpressionParsed.startsWith(m_q->prefix())){
            m_lastExpressionParsed.remove(0, m_q->prefix().size());
        }
        // Parse
        bool ok;
        ValueType value = parseMathExpression(m_lastExpressionParsed, &ok);
        // Validate
        if (!ok) {
            m_isLastValid = false;
            value = m_q->value();
            showWarning(showWarningInterval);
            emit m_q->errorWhileParsing(text);
        } else {
            if (!m_isLastValid) {
                m_isLastValid = true;
                emit m_q->noMoreParsingError();
            }
        }
        return value;
    }

    QString textFromValue(ValueType value) const
    {
        // If the last expression parsed is not null then the user actively
        // changed the text, so that expression is returned regardless of the
        // actual value
        if (!m_lastExpressionParsed.isNull()) {
            return m_lastExpressionParsed;
        }
        // Otherwise we transform the passed value to a string using the 
        // method from the base class and return that
        return m_q->BaseSpinBoxType::textFromValue(value);
    }

    // Fix the selection so that the start and the end are in the value text
    // and not in the prefix or suffix. This makes those unselectable
    void fixupSelection()
    {
        // If theres no selection just do nothing
        if (m_lineEdit->selectionStart() == -1 || m_lineEdit->selectionEnd() == -1) {
            return;
        }
        const int suffixStart = m_q->text().length() - m_q->suffix().length();
        const int newStart = qBound(m_q->prefix().length(), m_lineEdit->selectionStart(), suffixStart);
        const int newEnd = qBound(m_q->prefix().length(), m_lineEdit->selectionEnd(), suffixStart);
        if (m_lineEdit->cursorPosition() == m_lineEdit->selectionStart()) {
            m_lineEdit->setSelection(newEnd, -(newEnd - newStart));
        } else {
            m_lineEdit->setSelection(newStart, newEnd - newStart);
        }
    }

    // Fix the cursor position so that it is in the value text
    // and not in the prefix or suffix.
    void fixupCursorPosition(int oldPos, int newPos)
    {
        Q_UNUSED(oldPos);
        if (newPos < m_q->prefix().length()) {
            m_lineEdit->setCursorPosition(m_q->prefix().length());
        } else {
            const int suffixStart = m_q->text().length() - m_q->suffix().length();
            if (newPos > suffixStart) {
                m_lineEdit->setCursorPosition(suffixStart);
            }
        }
    }

    // Inmediately show the warning overlay and icon
    void showWarning() const
    {
        if (m_isWarningActive && m_warningAnimation.state() == QVariantAnimation::Running) {
            return;
        }
        m_timerShowWarning.stop();
        m_warningAnimation.stop();
        m_isWarningActive = true;
        if (!m_warningIcon.isNull()) {
            QFontMetricsF fm(m_lineEdit->font());
#if QT_VERSION >= QT_VERSION_CHECK(5,11,0)
            const qreal textWidth = fm.horizontalAdvance(m_lineEdit->text());
#else
            const qreal textWidth = fm.width(m_lineEdit->text());
#endif
            const int minimumWidth =
                static_cast<int>(
                    std::ceil(
                        textWidth + (m_q->alignment() == Qt::AlignCenter ? 2.0 : 1.0) * widthOfWarningIconArea + 4
                    )
                );
            if (m_lineEdit->width() >= minimumWidth) {
                m_showWarningIcon = true;
            } else {
                m_showWarningIcon = false;
            }
        }
        // scale the animation duration in case the animation is in the middle
        const int animationDuration =
            static_cast<int>(std::round((1.0 - m_warningAnimation.currentValue().toReal()) * warningAnimationDuration));
        m_warningAnimation.setStartValue(m_warningAnimation.currentValue());
        m_warningAnimation.setEndValue(1.0);
        m_warningAnimation.setDuration(animationDuration);
        m_warningAnimation.start();
    }

    // Show the warning after a specific amount of time
    void showWarning(int delay) const
    {
        if (delay > 0) {
            if (!m_isWarningActive || m_warningAnimation.state() != QVariantAnimation::Running) {
                m_timerShowWarning.start(delay);
            }
            return;
        }
        // If "delay" is not greater that 0 then the warning will be
        // inmediately shown
        showWarning();
    }

    void hideWarning() const
    {
        m_timerShowWarning.stop();
        m_warningAnimation.stop();
        m_isWarningActive = false;
        // scale the animation duration in case the animation is in the middle
        const int animationDuration =
            static_cast<int>(std::round(m_warningAnimation.currentValue().toReal() * warningAnimationDuration));
        m_warningAnimation.setStartValue(m_warningAnimation.currentValue());
        m_warningAnimation.setEndValue(0.0);
        m_warningAnimation.setDuration(animationDuration);
        m_warningAnimation.start();
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
        // spinbox, without overlaping the buttons
        QString style = qApp->property(currentUnderlyingStyleNameProperty).toString().toLower();
        if (style == "breeze") {
            rect.adjust(-4, -4, 0, 4);
        } else if (style == "fusion") {
            rect.adjust(-2, -1, 2, 1);
        }
        // Set the rect
        m_lineEdit->setGeometry(rect);

        return true;
    }

    bool qStyleChangeEvent(QEvent*)
    {
        // Fire a resize event so that the line edit geometry is updated.
        // For some reason (stylesheet set in the app) setting the geometry
        // using qstyle to get a rect has no effect here, as if the style is
        // not updated yet... 
        qApp->postEvent(m_q, new QResizeEvent(m_q->size(), m_q->size()));
        return false;
    }

    bool qKeyPressEvent(QKeyEvent *e)
    {
        switch (e->key()) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
                if (!isLastValid()) {
                    // Inmediately show the warning if the expression is not valid
                    showWarning();
                    return true;
                } else {
                    // Set the value forcing the expression to be overwritten.
                    // This will make an expression like "2*4" automaticaly be changed
                    // to "8" when enter/return key is pressed
                    setValue(m_q->value(), true);
                }
                break;
            // Prevent deleting the last character of the prefix and the first
            // one of the suffix. This solves some issue that apprears when the
            // prefix ends with a space or the suffix starts with a space. For
            // example, if the prefix is "size: " and the value 50, deleting
            // the space will join the string "size:" with "50" to form
            // "size:50", and since that string does not start with the prefix,
            // it will be treated as the new entered value. Then, prepending
            // the prefix will display the text "size: size:50".
            case Qt::Key_Backspace:
                if (m_lineEdit->selectionLength() == 0 && m_lineEdit->cursorPosition() == m_q->prefix().length()) {
                    return true;
                }
                break;
            case Qt::Key_Delete:
                if (m_lineEdit->selectionLength() == 0 && m_lineEdit->cursorPosition() == m_q->text().length() - m_q->suffix().length()) {
                    return true;
                }
                break;
            default:
                break;
        }
        return false;
    }

    bool qFocusOutEvent(QFocusEvent*)
    {
        if (!isLastValid()) {
            // Inmediately show the warning if the expression is not valid
            showWarning();
        } else {
            // Set the value forcing the expression to be overwritten.
            // This will make an expression like "2*4" automaticaly be changed
            // to "8" when the spinbox looses focus 
            setValue(m_q->value(), true);
        }
        return false;
    }

    bool lineEditPaintEvent(QPaintEvent*)
    {
        QPainter painter(m_lineEdit);
        painter.setRenderHint(QPainter::Antialiasing, true);
        QPalette pal = m_lineEdit->palette();
        // the overlay color, a red warning color when there is an error
        QColor color(255, 48, 0, 0);
        constexpr int maxOpacity = 160;
        QColor textColor;
        const qreal warningAnimationPos = m_warningAnimation.currentValue().toReal();
        // compute colors
        if (m_warningAnimation.state() == QVariantAnimation::Running) {
            color.setAlpha(static_cast<int>(std::round(KisAlgebra2D::lerp(0.0, static_cast<double>(maxOpacity), warningAnimationPos))));
            textColor = KisPaintingTweaks::blendColors(m_q->palette().text().color(), Qt::white, 1.0 - warningAnimationPos);
        } else {
            if (m_isWarningActive) {
                color.setAlpha(maxOpacity);
                textColor = Qt::white;
            } else {
                textColor = m_q->palette().text().color();
            }
        }
        // Paint the overlay
        const QRect rect = m_lineEdit->rect();
        painter.setBrush(color);
        painter.setPen(Qt::NoPen);
        QString style = qApp->property(currentUnderlyingStyleNameProperty).toString().toLower();
        if (style == "fusion") {
            painter.drawRoundedRect(rect, 1, 1);
        } else {
            painter.drawRoundedRect(rect, 0, 0);
        }
        // Paint the warning icon
        if (m_showWarningIcon) {
            constexpr qreal warningIconMargin = 4.0;
            const qreal warningIconSize = widthOfWarningIconArea - 2.0 * warningIconMargin;
            if (m_warningAnimation.state() == QVariantAnimation::Running) {
                qreal warningIconPos = KisAlgebra2D::lerp(-warningIconMargin, warningIconMargin, warningAnimationPos);
                painter.setOpacity(warningAnimationPos);
                painter.drawPixmap(
                    warningIconPos, (static_cast<qreal>(rect.height()) - warningIconSize) / 2.0,
                    m_warningIcon.pixmap(warningIconSize, warningIconSize)
                );
            } else if (m_isWarningActive) {
                painter.drawPixmap(
                    warningIconMargin, (static_cast<qreal>(rect.height()) - warningIconSize) / 2.0,
                    m_warningIcon.pixmap(warningIconSize, warningIconSize)
                );
            }
        }
        // Set the text color
        pal.setBrush(QPalette::Text, textColor);
        // Make sure the background of the line edit is transparent so that
        // the base class paint event only draws the text
        pal.setBrush(QPalette::Base, Qt::transparent);
        pal.setBrush(QPalette::Button, Qt::transparent);
        m_lineEdit->setPalette(pal);
        return false;
    }

    bool lineEditMouseDoubleClickEvent(QMouseEvent *e)
    {
        if (!m_q->isEnabled() || m_lineEdit->isReadOnly()) {
            return false;
        }
        // If we double click anywhere with the left button then select all the value text
        if (e->button() == Qt::LeftButton) {
            m_q->selectAll();
            return true;
        }
        return false;
    }

    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (!o || !e) {
            return false;
        }
        if (o == m_q) {
            switch (e->type()) {
                case QEvent::Resize: return qResizeEvent(static_cast<QResizeEvent*>(e));
                case QEvent::StyleChange: return qStyleChangeEvent(e);
                case QEvent::KeyPress: return qKeyPressEvent(static_cast<QKeyEvent*>(e));
                case QEvent::FocusOut: return qFocusOutEvent(static_cast<QFocusEvent*>(e));
                default: break;
            }
        } else if (o == m_lineEdit) {
            switch (e->type()) {
                case QEvent::Paint: return lineEditPaintEvent(static_cast<QPaintEvent*>(e));
                case QEvent::MouseButtonDblClick: return lineEditMouseDoubleClickEvent(static_cast<QMouseEvent*>(e));
                default: break;
            }
        }
        return false;
    }

private:
    // Amount of time that has to pass after a keypress to show
    // the warning, in milliseconds
    static constexpr int showWarningInterval{2000};
    // The width of the warning icon
    static constexpr double widthOfWarningIconArea{24.0};
    // The animation duration
    static constexpr double warningAnimationDuration{250.0};

    SpinBoxType *m_q;
    QLineEdit *m_lineEdit;
    mutable QString m_lastExpressionParsed;
    mutable bool m_isLastValid{true};
    mutable bool m_isWarningActive{false};
    mutable QTimer m_timerShowWarning;
    mutable bool m_showWarningIcon{false};
    mutable QVariantAnimation m_warningAnimation;
    static QIcon m_warningIcon;
};

template <typename SpinBoxTypeTP, typename BaseSpinBoxTypeTP>
QIcon KisParseSpinBoxPrivate<SpinBoxTypeTP, BaseSpinBoxTypeTP>::m_warningIcon;

#endif // KISPARSESPINBOXPRIVATE_H
