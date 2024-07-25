/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KIS_SPECIFIC_COLOR_SELECTOR_WIDGET_H_
#define _KIS_SPECIFIC_COLOR_SELECTOR_WIDGET_H_

#include <QWidget>

#include <KoColor.h>
#include <QtWidgets/QComboBox>
#include "kis_signal_auto_connection.h"

#include "ui_wdgSpecificColorSelectorWidget.h"


class KoColorSpace;
class QVBoxLayout;
class KisColorInput;
class KisColorSpaceSelector;
class QButtonGroup;
class QRadioButton;
class QAbstractButton;
class KisSignalCompressor;
class QSpacerItem;
class KisDisplayColorConverter;
class KisPopupButton;
class KisHexColorInput;
class KisHsvColorInput;

class KisSpecificColorSelectorWidget : public QWidget
{
    Q_OBJECT
public:
    KisSpecificColorSelectorWidget(QWidget* parent);
    ~KisSpecificColorSelectorWidget() override;

protected:
    void resizeEvent(QResizeEvent* event) override;

public Q_SLOTS:
    void setDisplayConverter(KisDisplayColorConverter *colorConverter);

    void setColorSpace(const KoColorSpace *cs, bool force = false);
    void setColor(const KoColor&);
    void setFGColor(const KoColor& c);

private Q_SLOTS:
    void update();
    void updateTimeout();
    void setCustomColorSpace(const KoColorSpace *);
    void setUseSameColorSpace(bool locked, bool reloadColorSpace = true);
    void rereadCurrentColorSpace(bool force = false);
    void onChkUsePercentageChanged(bool);
    void hsvSelectorClicked(QAbstractButton *);
    void changeHsxMode(int index);

Q_SIGNALS:
    void colorChanged(const KoColor&);
    void updated();

private:
    void updateHsvSelector(bool isRgbColorSpace);

    QList<KisColorInput *> m_inputs;
    KisHexColorInput *m_hexInput;
    KisHsvColorInput *m_hsvSlider;
    QRadioButton *m_rgbButton;
    QRadioButton *m_hsxButton;

    QButtonGroup *m_hsvSelector;
    const KoColorSpace* m_colorSpace;
    KoColor m_color;
    KoColor m_FGColor;
    bool m_updateAllowed;
    KisSignalCompressor *m_updateCompressor;
    KisColorSpaceSelector *m_colorspaceSelector;
    QScopedPointer<Ui_wdgSpecificColorSelectorWidget> m_ui;

    KisDisplayColorConverter *m_displayConverter;
    KisSignalAutoConnectionsStore m_converterConnection;

    QComboBox* m_hsxModeComboBox;
};

#endif
