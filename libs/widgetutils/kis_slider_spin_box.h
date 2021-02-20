/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2010 Justin Noel <justin@ics.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KISSLIDERSPINBOX_H
#define KISSLIDERSPINBOX_H

#include <QAbstractSpinBox>

#include <QStyleOptionSpinBox>
#include <QStyleOptionProgressBar>
#include <kritawidgetutils_export.h>

class KisAbstractSliderSpinBoxPrivate;
class KisSliderSpinBoxPrivate;
class KisDoubleSliderSpinBoxPrivate;

/**
 * XXX: when inactive, also show the progress bar part as inactive!
 */
class KRITAWIDGETUTILS_EXPORT KisAbstractSliderSpinBox : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(KisAbstractSliderSpinBox)
    Q_DECLARE_PRIVATE(KisAbstractSliderSpinBox)
protected:
    explicit KisAbstractSliderSpinBox(QWidget* parent, KisAbstractSliderSpinBoxPrivate*);
public:
    ~KisAbstractSliderSpinBox() override;

    void showEdit();
    void hideEdit();

    void setPrefix(const QString& prefix);
    void setSuffix(const QString& suffix);

    void setExponentRatio(qreal dbl);

    /**
     * If set to block, it informs inheriting classes that they shouldn't emit signals
     * if the update comes from a mouse dragging the slider.
     * Set this to true when dragging the slider and updates during the drag are not needed.
     */
    void setBlockUpdateSignalOnDrag(bool block);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    virtual QSize minimumSize() const;

    bool isDragging() const;

protected:
    void paintEvent(QPaintEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void wheelEvent(QWheelEvent *) override;
    bool event(QEvent *event) override;

    bool eventFilter(QObject* recv, QEvent* e) override;

    QStyleOptionSpinBox spinBoxOptions() const;
    QStyleOptionProgressBar progressBarOptions() const;

    QRect progressRect(const QStyleOptionSpinBox& spinBoxOptions) const;
    QRect upButtonRect(const QStyleOptionSpinBox& spinBoxOptions) const;
    QRect downButtonRect(const QStyleOptionSpinBox& spinBoxOptions) const;

    int valueForX(int x, Qt::KeyboardModifiers modifiers = Qt::NoModifier) const;

    void commitEnteredValue();

    virtual QString valueString() const = 0;
    /**
     * Sets the slider internal value. Inheriting classes should respect blockUpdateSignal
     * so that, in specific cases, we have a performance improvement. See setIgnoreMouseMoveEvents.
     */
    virtual void setInternalValue(int value, bool blockUpdateSignal) = 0;

    /**
     * Allows inheriting classes to directly set the value
     */
    void setPrivateValue(int value);

protected Q_SLOTS:
    void contextMenuEvent(QContextMenuEvent * event) override;
    void editLostFocus();
protected:
    KisAbstractSliderSpinBoxPrivate* const d_ptr;

    // QWidget interface
protected:
    void changeEvent(QEvent *e) override;
    void paintSlider(QPainter& painter);

private:
    void setInternalValue(int value);
};

class KRITAWIDGETUTILS_EXPORT KisSliderSpinBox : public KisAbstractSliderSpinBox
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(KisSliderSpinBox)
    Q_PROPERTY( int minimum READ minimum WRITE setMinimum )
    Q_PROPERTY( int maximum READ maximum WRITE setMaximum )
public:
    KisSliderSpinBox(QWidget* parent = 0);
    ~KisSliderSpinBox() override;

    void setRange(int minimum, int maximum);

    int minimum() const;
    void setMinimum(int minimum);
    int maximum() const;
    void setMaximum(int maximum);
    int fastSliderStep() const;
    void setFastSliderStep(int step);

    ///Get the value, don't use value()
    int value();

    void setSingleStep(int value);
    void setPageStep(int value);

public Q_SLOTS:

    ///Set the value, don't use setValue()
    void setValue(int value);

protected:
    QString valueString() const override;
    void setInternalValue(int value, bool blockUpdateSignal) override;
Q_SIGNALS:
    void valueChanged(int value);
};

class KRITAWIDGETUTILS_EXPORT KisDoubleSliderSpinBox : public KisAbstractSliderSpinBox
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(KisDoubleSliderSpinBox)
public:
    KisDoubleSliderSpinBox(QWidget* parent = 0);
    ~KisDoubleSliderSpinBox() override;

    void setRange(qreal minimum, qreal maximum, int decimals = 0);

    qreal minimum() const;
    void setMinimum(qreal minimum);
    qreal maximum() const;
    void setMaximum(qreal maximum);
    qreal fastSliderStep() const;
    void setFastSliderStep(qreal step);

    qreal value();
    void setSingleStep(qreal value);

public Q_SLOTS:
    void setValue(qreal value);

protected:
    QString valueString() const override;
    void setInternalValue(int value, bool blockUpdateSignal) override;
Q_SIGNALS:
    void valueChanged(qreal value);
};

#endif //kISSLIDERSPINBOX_H
