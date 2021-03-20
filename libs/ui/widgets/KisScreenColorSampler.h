/*
 * SPDX-FileCopyrightText: 2016 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSCREENCOLORSAMPLER_H
#define KISSCREENCOLORSAMPLER_H

#include <QScopedPointer>
#include <QEvent>
#include <QMouseEvent>

#include "KoColor.h"
#include <KisScreenColorSamplerBase.h>

#include "kritaui_export.h"

/**
 * @brief The KisScreenColorSampler class
 * Based on the original QColorDialog's screen color picker, this class provides a button
 * that can be used to activate a color sampler that can sample from anywhere on the screen.
 */
class KRITAUI_EXPORT KisScreenColorSampler : public KisScreenColorSamplerBase
{
    Q_OBJECT
public:
    explicit KisScreenColorSampler(bool showInfoLabel = false, QWidget *parent = 0);
    ~KisScreenColorSampler() override;

    KoColor currentColor();

    bool handleColorSamplingMouseMove(QMouseEvent *e);
    bool handleColorSamplingMouseButtonRelease(QMouseEvent *e);
    bool handleColorSamplingKeyPress(QKeyEvent *e);

    /// reloads icon(s) when theme is updated
    void updateIcons() override;

    static KisScreenColorSampler *createScreenColorSampler(QWidget *parent = 0) {return new KisScreenColorSampler(parent);}

Q_SIGNALS:
    void sigNewColorSampled(KoColor c);

public Q_SLOTS:
    void sampleScreenColor();

private Q_SLOTS:
    void updateColorSampling();
protected:
    void changeEvent(QEvent *event) override;
private:
    struct Private; //The private struct
    const QScopedPointer<Private> m_d; //the private pointer

    void setCurrentColor(KoColor c);
    KoColor grabScreenColor(const QPoint &p);
    void updateColorLabelText(const QPoint &globalPos);
    void releaseColorSampling();
    void continueUpdateColorSampling(const QPoint &globalPos);
};

class KisScreenColorSamplingEventFilter : public QObject {
public:
    explicit KisScreenColorSamplingEventFilter(KisScreenColorSampler *w, QObject *parent = 0);

    bool eventFilter(QObject *, QEvent *event) override;
private:
    KisScreenColorSampler *m_w;
};

#endif // KISSCREENCOLORSAMPLER_H
