/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
    QScopedPointer<Ui_wdgSpecificColorSelectorWidget> m_ui;


    KisDisplayColorConverter *m_displayConverter;
    KisSignalAutoConnectionsStore m_converterConnection;
};

#endif
