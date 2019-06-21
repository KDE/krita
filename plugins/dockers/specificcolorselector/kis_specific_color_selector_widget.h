/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
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

#ifndef _KIS_SPECIFIC_COLOR_SELECTOR_WIDGET_H_
#define _KIS_SPECIFIC_COLOR_SELECTOR_WIDGET_H_

#include <QWidget>

#include <KoColor.h>
#include "kis_signal_auto_connection.h"

#include "ui_wdgSpecificColorSelectorWidget.h"


class KoColorSpace;
class QVBoxLayout;
class KisColorInput;
class KisColorSpaceSelector;
class QCheckBox;
class KisSignalCompressor;
class QSpacerItem;
class KisDisplayColorConverter;
class KisPopupButton;

class KisSpecificColorSelectorWidget : public QWidget
{
    Q_OBJECT
public:
    KisSpecificColorSelectorWidget(QWidget* parent);
    ~KisSpecificColorSelectorWidget() override;
    bool customColorSpaceUsed();

protected:
    void resizeEvent(QResizeEvent* event) override;

public Q_SLOTS:
    void setDisplayConverter(KisDisplayColorConverter *colorConverter);

    void setColorSpace(const KoColorSpace *cs, bool force = false);
    void setColor(const KoColor&);
private Q_SLOTS:
    void update();
    void updateTimeout();
    void setCustomColorSpace(const KoColorSpace *);
    void rereadCurrentColorSpace(bool force = false);
    void onChkUsePercentageChanged(bool);
Q_SIGNALS:
    void colorChanged(const KoColor&);
    void updated();
private:
    QList<KisColorInput*> m_inputs;
    const KoColorSpace* m_colorSpace;
    QSpacerItem *m_spacer;
    KoColor m_color;
    bool m_updateAllowed;
    KisSignalCompressor *m_updateCompressor;
    KisColorSpaceSelector *m_colorspaceSelector;
    bool m_customColorSpaceSelected;
    Ui_wdgSpecificColorSelectorWidget* m_ui;


    KisDisplayColorConverter *m_displayConverter;
    KisSignalAutoConnectionsStore m_converterConnection;
};

#endif
