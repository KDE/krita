/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2011 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *  SPDX-FileCopyrightText: 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_ADVANCED_COLOR_SPACE_SELECTOR_H_
#define _KIS_ADVANCED_COLOR_SPACE_SELECTOR_H_

#include <QWidget>
#include <QDialog>
#include "kritaui_export.h"

#include "ui_wdgcolorspaceselectoradvanced.h"

class KoID;
class KoColorSpace;

/* Use KisColorSpaceSelector instead of directly using this one*/

class KRITAUI_EXPORT KisAdvancedColorSpaceSelector : public QDialog
{
    Q_OBJECT
public:
    KisAdvancedColorSpaceSelector(QWidget* parent, const QString &caption);
    ~KisAdvancedColorSpaceSelector() override;
    const KoColorSpace* currentColorSpace();
    void setCurrentColorModel(const KoID& id);
    void setCurrentColorDepth(const KoID& id);
    void setCurrentProfile(const QString& name);
    void setCurrentColorSpace(const KoColorSpace* colorSpace);
Q_SIGNALS:
    void selectionChanged(bool valid);
    void colorSpaceChanged(const KoColorSpace*);
private Q_SLOTS:
    void fillCmbDepths(const KoID& idd);
    void fillLstProfiles();
    void fillDescription();
    QString nameWhitePoint(QVector <double> whitePoint);
    void colorSpaceChanged();
    void installProfile();
private:
    struct Private;
    Private * const d;
};


#endif
