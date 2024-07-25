/*
 * SPDX-FileCopyrightText: 2018 Jouni Pentikäinen <joupent@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISSELECTIONPROPERTYSLIDER_H
#define KISSELECTIONPROPERTYSLIDER_H

#include <QObject>

#include <KoShape.h>
#include <kritaui_export.h>
#include <kis_signals_blocker.h>
#include "kis_slider_spin_box.h"

class KRITAUI_EXPORT KisSelectionPropertySliderBase : public KisDoubleSliderSpinBox
{
    Q_OBJECT

public:
    explicit KisSelectionPropertySliderBase(QWidget* parent = 0);
    ~KisSelectionPropertySliderBase() override;

    /**
     * Set the prefix/suffix using i18n strings in the form of `prefix {n} suffix`.
     *
     * @param normalTemplate The text in the form of `prefix{n}suffix`, usually
     *                       passed through `i18n` or `i18nc`, for when there
     *                       is only one selection or multiple selections with
     *                       the same value.
     * @param mixedTemplate The text in the form of `prefix{n}suffix`, usually
     *                      passed through `i18n` or `i18nc`, for when there
     *                      are multiple selections with different values.
     */
    void setTextTemplates(const QString &normalTemplate, const QString &mixedTemplate);

    /**
     * **Deleted function** - use `setTextTemplates` instead.
     */
    void setPrefix(const QString &) = delete;

    /**
     * **Deleted function** - use `setTextTemplates` instead.
     */
    void setSuffix(const QString &) = delete;

protected:
    void setInternalValue(qreal value, bool blockUpdateSignal) override;

    void setSelectionValue(qreal commonValue, bool mixed);

    virtual bool hasSelection() const = 0;
    virtual qreal getCommonValue() const = 0;

private Q_SLOTS:
    void slotCompressedUpdate();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

/**
 * This is a generic slider for adjusting a property across a set of one or
 * more items such as a selection.
 *
 * When using this class, first call the setValueGetter method to allow the
 * slider to get values from the items. For example:
 * slider->setValueGetter(
 *     [](KoShape *s) { return s->transparency(); }
 * );
 *
 * To update the slider, call setSelection with the new set of objects.
 *
 * When the slider is dragged, valueChanged(qreal) signals are emitted after
 * signal compression.
 */
template<class T>
class KRITAUI_EXPORT KisSelectionPropertySlider : public KisSelectionPropertySliderBase
{
public:
    explicit KisSelectionPropertySlider(QWidget *parent = 0)
        : KisSelectionPropertySliderBase(parent)
    {}

    void setValueGetter(qreal (*getter)(T))
    {
        m_valueGetter = getter;
    }

    void setSelection(QList<T> newSelection)
    {
        KisSignalsBlocker b(this);

        m_selection = newSelection;

        const qreal commonValue = getCommonValue();

        setEnabled(!m_selection.isEmpty());
        setSelectionValue(commonValue, commonValue < 0.0);
    }

    QList<T> selection() const {
        return m_selection;
    }

protected:
    bool hasSelection() const override
    {
        return !m_selection.isEmpty();
    }

    qreal getCommonValue() const override
    {
        qreal commonValue = -1.0;

        Q_FOREACH (T item, m_selection) {
            const qreal itemValue = m_valueGetter(item);

            if (commonValue < 0) {
                commonValue = itemValue;
            } else if (!qFuzzyCompare(commonValue, itemValue)) {
                commonValue = -1.0;
                break;
            }
        }

        return commonValue;
    }

private:
    qreal (*m_valueGetter)(T) {nullptr};
    QList<T> m_selection;
};

class KRITAUI_EXPORT KisShapePropertySlider : public KisSelectionPropertySlider<KoShape*>
{
public:
    KisShapePropertySlider(QWidget* parent=nullptr);
};

#endif
