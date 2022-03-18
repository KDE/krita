/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KIS_COLOR_INPUT_H_
#define _KIS_COLOR_INPUT_H_

#include <QWidget>

class KoChannelInfo;
class KoColor;
class QWidget;
class QSpinBox;
class QDoubleSpinBox;
class KisIntParseSpinBox;
class KisDoubleParseSpinBox;
class KoColorSlider;
class QLineEdit;
class QLabel;

#include <KoColorDisplayRendererInterface.h>
#include "kritawidgets_export.h"

class KRITAWIDGETS_EXPORT KisColorInput : public QWidget
{
    Q_OBJECT
public:
    KisColorInput(QWidget* parent, const KoChannelInfo*, KoColor* color, KoColorDisplayRendererInterface *displayRenderer = KoDumbColorDisplayRenderer::instance(), bool usePercentage = false);
    inline bool usePercentage() const {
        return m_usePercentage;
    }
    virtual inline void setPercentageWise(bool val) {
        m_usePercentage = val;
    }

protected:
    void init();
    virtual QWidget* createInput() = 0;
Q_SIGNALS:
    void updated();
protected:
    const KoChannelInfo* m_channelInfo {nullptr};
    KoColor* m_color {nullptr};
    KoColorSlider* m_colorSlider {nullptr};
    KoColorDisplayRendererInterface *m_displayRenderer {nullptr};
    bool m_usePercentage {false};
};

class KRITAWIDGETS_EXPORT KisIntegerColorInput : public KisColorInput
{
    Q_OBJECT
public:
    KisIntegerColorInput(QWidget* parent, const KoChannelInfo*, KoColor* color, KoColorDisplayRendererInterface *displayRenderer = KoDumbColorDisplayRenderer::instance(), bool usePercentage = false);
protected:
    QWidget* createInput() override;
    void setPercentageWise(bool val) override;
public Q_SLOTS:
    void setValue(int);
    void update();
private Q_SLOTS:
    void onColorSliderChanged(int);
    void onNumInputChanged(int);
private:
    KisIntParseSpinBox* m_intNumInput {nullptr};
};


class KRITAWIDGETS_EXPORT KisFloatColorInput : public KisColorInput
{
    Q_OBJECT
public:
    KisFloatColorInput(QWidget* parent, const KoChannelInfo*, KoColor* color, KoColorDisplayRendererInterface *displayRenderer = KoDumbColorDisplayRenderer::instance(), bool usePercentage = false);
protected:
    QWidget* createInput() override;
public Q_SLOTS:
    void setValue(double);
    void sliderChanged(int);
    void update();
private:
    KisDoubleParseSpinBox* m_dblNumInput {nullptr};
    qreal m_minValue {0.0};
    qreal m_maxValue {0.0};
};

class KRITAWIDGETS_EXPORT KisHexColorInput : public KisColorInput
{
    Q_OBJECT
public:
    KisHexColorInput(QWidget* parent, KoColor* color, KoColorDisplayRendererInterface *displayRenderer = KoDumbColorDisplayRenderer::instance(), bool usePercentage = false, bool usePreview = false);
protected:
    QWidget* createInput() override;
public Q_SLOTS:
    void setValue();
    void update();
private:
    QLineEdit* m_hexInput {nullptr};
    QLabel* m_colorPreview=nullptr;
};

#endif
