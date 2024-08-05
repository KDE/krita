/*
 * SPDX-FileCopyrightText: 2016 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 * SPDX-FileCopyrightText: 2022 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef KIS_VISUAL_COLOR_MODEL_H
#define KIS_VISUAL_COLOR_MODEL_H

#include <QObject>
#include <QScopedPointer>

#include <KoColor.h>

#include "kritawidgets_export.h"

class KoColorSpace;
class KoColorDisplayRendererInterface;

/**
 * @brief The KisVisualColorModel class allows manipulating a KoColor using various color models
 *
 * This enables different widgets to easily manipulate the same color representation
 * (like HSV channels of an RGB color etc.) in a synchronized way without having to know
 * the details of the underlying color space.
 * NOTE: This class is meant to be shared between GUI classes using KisVisualColorModelSP,
 * so DO NOT SET a QObject parent in this case.
 */

class KRITAWIDGETS_EXPORT KisVisualColorModel : public QObject
{
    Q_OBJECT
public:
    enum ColorModel { None, Channel, HSV, HSL, HSI, HSY, YUV };

    KisVisualColorModel();
    ~KisVisualColorModel() override;

    KoColor currentColor() const;
    QVector4D channelValues() const;
    int colorChannelCount() const;
    ColorModel colorModel() const;
    QVector4D maxChannelValues() const;
    void setMaxChannelValues(const QVector4D &maxValues);
    /**
     * @brief Copy the internal state of another KisVisualColorModel
     *
     * This will Q_EMIT all necessary signals to update widgets so they
     * can reflect the new state correctly.
     *
     * @param other the model to copy
     */
    void copyState(const KisVisualColorModel &other);
    /**
     * @brief Set the HSX color model used for RGB color spaces.
     */
    void setRGBColorModel(ColorModel model);
    const KoColorSpace* colorSpace() const;
    bool isHSXModel() const;
    bool supportsExposure() const;
    KoColor convertChannelValuesToKoColor(const QVector4D &values) const;
    QVector4D convertKoColorToChannelValues(KoColor c) const;

public Q_SLOTS:

    void slotSetColor(const KoColor &c);
    void slotSetColorSpace(const KoColorSpace *cs);
    void slotSetChannelValues(const QVector4D &values);
    /**
     * @brief slotLoadACSConfig loads supported settings from Advanced Color Selector
     */
    void slotLoadACSConfig();

private:
    void loadColorSpace(const KoColorSpace *cs);
    void emitChannelValues();

Q_SIGNALS:
    /**
     * @brief sigColorModelChanged is emitted whenever the color model changes.
     *
     * This can either be due to a change in color space that uses a different channel
     * model, or when the same color space gets represented in a different way,
     * for example RGB representation switches from HSV to HSL etc. so the values of
     * sigChannelValuesChanged() change meaning.
     * Note: This will be followed by sigChannelValuesChanged() unless caused by a
     * color space change, new channel values are not available yet when this signal occurs.
     *
     * @see getColorModel()
     */
    void sigColorModelChanged();
    /**
     * @brief sigColorSpaceChanged notifies that the color space from which the
     *        channel values are derived changed, and thus invalidating the current ones.
     */
    void sigColorSpaceChanged();
    void sigChannelValuesChanged(const QVector4D &values, quint32 channelFlags);
    void sigNewColor(const KoColor &c);

private:
    struct Private;
    const QScopedPointer<Private> m_d;

};

typedef QSharedPointer<KisVisualColorModel> KisVisualColorModelSP;

#endif // KIS_VISUAL_COLOR_MODEL_H
