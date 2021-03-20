/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_COLOR_SPACE_SELECTOR_H_
#define _KIS_COLOR_SPACE_SELECTOR_H_

#include <QWidget>
#include <kritaui_export.h>

class KoID;
class KoColorSpace;
class KisAdvancedColorSpaceSelector;

class KRITAUI_EXPORT KisColorSpaceSelector : public QWidget
{
    Q_OBJECT
public:
    KisColorSpaceSelector(QWidget* parent);
    ~KisColorSpaceSelector() override;
    const KoColorSpace* currentColorSpace();
    void setCurrentColorModel(const KoID& id);
    void setCurrentColorDepth(const KoID& id);
    void setCurrentProfile(const QString& name);
    void setCurrentColorSpace(const KoColorSpace* colorSpace);
    void showColorBrowserButton(bool showButton);
Q_SIGNALS:
    /**
     * This signal is emitted when a new color space is selected.
     * @param valid indicates if the color space can be used
     */
    void selectionChanged(bool valid);
    /// This signal is emitted, when a new color space is selected, that can be used (eg is valid)
    void colorSpaceChanged(const KoColorSpace*);
private Q_SLOTS:
    void fillCmbDepths(const KoID& idd);
    void fillCmbProfiles();
    void colorSpaceChanged();
    void installProfile();
    void slotOpenAdvancedSelector();
    void slotProfileValid(bool valid);

    void slotModelsComboBoxActivated(const KoID& id);
    void slotDepthsComboBoxActivated();
    void slotProfilesComboBoxActivated();
private:
    struct Private;
    KisAdvancedColorSpaceSelector *m_advancedSelector;
    Private * const d;

};


#endif
