/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KIS_SMALL_COLOR_WIDGET_H_
#define _KIS_SMALL_COLOR_WIDGET_H_

#include <QWidget>

class KoColor;
class KisDisplayColorConverter;
class KisGLImageWidget;

class KisSmallColorWidget : public QWidget
{
    Q_OBJECT
public:
    KisSmallColorWidget(QWidget* parent);
    ~KisSmallColorWidget() override;
public:
    void resizeEvent(QResizeEvent * event) override;

    void setDisplayColorConverter(KisDisplayColorConverter *converter);

public:

public Q_SLOTS:
    void setHue(qreal h);
    void setHSV(qreal h, qreal s, qreal v, bool notifyChanged = true);
    void setColor(const KoColor &color);

    void slotUpdatePalettes();
    void updateSVPalette();

Q_SIGNALS:
    void colorChanged(const KoColor&);

    void sigTellColorChangedInternal();

private Q_SLOTS:
    void slotHueSliderChanged(const QPointF &pos);
    void slotValueSliderChanged(const QPointF &pos);
    void slotInitiateUpdateDynamicRange(int maxLuminance);
    void slotDisplayConfigurationChanged();
    void slotTellColorChanged();

private:
    void updateDynamicRange(int maxLuminance);

private:

    void updateHuePalette();

    template<class FillPolicy>
    void uploadPaletteData(KisGLImageWidget *widget, const QSize &size);


private:
    struct Private;
    Private* const d;
};

#endif
