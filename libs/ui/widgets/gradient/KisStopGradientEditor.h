/*
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2016 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_STOP_GRADIENT_EDITOR_H_
#define _KIS_STOP_GRADIENT_EDITOR_H_

#include "kritaui_export.h"
#include "ui_wdgstopgradienteditor.h"
#include <boost/optional.hpp>
#include <KoStopGradient.h>

class KRITAUI_EXPORT KisStopGradientEditor : public QWidget, public Ui::KisWdgStopGradientEditor
{
    Q_OBJECT

public:
    enum SortFlag {
        SORT_ASCENDING = 1 << 0,
        EVEN_DISTRIBUTION = 1 << 1
    };
    Q_DECLARE_FLAGS( SortFlags, SortFlag);


    KisStopGradientEditor(QWidget *parent);
    KisStopGradientEditor(KoStopGradientSP gradient, QWidget *parent, const char* name, const QString& caption, KoCanvasResourcesInterfaceSP canvasResourcesInterface);

    void setCompactMode(bool value);

    void setGradient(KoStopGradientSP gradient);

    void setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface);
    KoCanvasResourcesInterfaceSP canvasResourcesInterface() const;

    void notifyGlobalColorChanged(const KoColor &color);

    boost::optional<KoColor> currentActiveStopColor() const;

Q_SIGNALS:
    void sigGradientChanged();

private:
     KoStopGradientSP m_gradient;
     KoCanvasResourcesInterfaceSP m_canvasResourcesInterface;

private Q_SLOTS:
    void stopChanged(int stop);
    void stopTypeChanged();
    void colorChanged(const KoColor& color);
    void opacityChanged(qreal value);
    void nameChanged();
    void reverse();
    void sortByValue(SortFlags flags);
    void showContextMenu( const class QPoint& origin );
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KisStopGradientEditor::SortFlags);
#endif
