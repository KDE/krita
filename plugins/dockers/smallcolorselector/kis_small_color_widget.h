/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
