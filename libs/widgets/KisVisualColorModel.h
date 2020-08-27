/*
 * SPDX-FileCopyrightText: 2016 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
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

class KRITAWIDGETS_EXPORT KisVisualColorModel : public QObject
{
    Q_OBJECT
public:
    enum ColorModel { None, Channel, HSV, HSL, HSI, HSY, YUV };

    explicit KisVisualColorModel(QObject *parent = 0);
    ~KisVisualColorModel() override;

    KoColor currentColor() const;
    QVector4D channelValues() const;
    int colorChannelCount() const;
    ColorModel colorModel() const;
    void setColorModel(ColorModel model);
    const KoColorSpace* colorSpace() const;
    const KoColorDisplayRendererInterface* displayRenderer() const;
    bool isHSXModel() const;
    KoColor convertChannelValuesToKoColor(const QVector4D &values) const;
    QVector4D convertKoColorToChannelValues(KoColor c) const;

public Q_SLOTS:

    void slotSetColor(const KoColor &c);
    void slotSetColorSpace(const KoColorSpace *cs);
    void slotSetChannelValues(const QVector4D &values);
    void setDisplayRenderer (const KoColorDisplayRendererInterface *displayRenderer);
    /**
     * @brief slotLoadACSConfig loads supported settings from Advanced Color Selector
     */
    void slotLoadACSConfig();

private Q_SLOTS:
    void slotDisplayConfigurationChanged();

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
    void sigChannelValuesChanged(const QVector4D &values);
    void sigNewColor(const KoColor &c);

private:
    struct Private;
    const QScopedPointer<Private> m_d;

};

#endif // KIS_VISUAL_COLOR_MODEL_H
