/*
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_COMPOSITEOP_OPTION_H
#define KIS_COMPOSITEOP_OPTION_H

#include <kis_paintop_option.h>
#include <kritapaintop_export.h>
#include <QString>

class QLabel;
class QModelIndex;
class QPushButton;
class KisCompositeOpListWidget;
class KoID;

class PAINTOP_EXPORT KisCompositeOpOption: public KisPaintOpOption
{
    Q_OBJECT

public:
    KisCompositeOpOption(bool createConfigWidget = false);
    ~KisCompositeOpOption() override;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private Q_SLOTS:
    void slotCompositeOpChanged(const QModelIndex& index);
    void slotEraserToggled(bool toggled);

private:
    void changeCompositeOp(const KoID& compositeOp);

private:
    QLabel*                   m_label;
    QPushButton*              m_bnEraser;
    KisCompositeOpListWidget* m_list;
    QString                   m_currCompositeOpID;
    bool                      m_createConfigWidget;
    bool                      m_eraserMode;

};

#endif
