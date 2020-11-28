/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_COLOR_SELECTOR_COMBO_BOX_H
#define KIS_COLOR_SELECTOR_COMBO_BOX_H

#include <QComboBox>
#include "kis_color_selector.h"

class KisColorSelectorComboBoxPrivate;
class KoColorSpace;

class KisColorSelectorComboBox : public QComboBox
{
    Q_OBJECT
public:
    KisColorSelectorComboBox(QWidget* parent=0);
    ~KisColorSelectorComboBox() override;
    void hidePopup() override;
    void showPopup() override;
    KisColorSelectorConfiguration configuration() const;
    //int m_model;
protected:
    void paintEvent(QPaintEvent *e) override;
public Q_SLOTS:
    void setColorSpace(const KoColorSpace* colorSpace);
    void setConfiguration(KisColorSelectorConfiguration);
    void setList(int model);
private:
    KisColorSelectorComboBoxPrivate* m_private;
    KisColorSelectorConfiguration m_configuration;
    KisColorSelector m_currentSelector;
};

#endif
