/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_TOOL_LAZY_BRUSH_OPTIONS_WIDGET_H
#define __KIS_TOOL_LAZY_BRUSH_OPTIONS_WIDGET_H

#include <QScopedPointer>
#include <QWidget>
#include <QModelIndex>

#include "kis_types.h"
#include "KisSwatchGroup.h"

class KisCanvasResourceProvider;
class KoColor;


/**
 * @brief The KisToolLazyBrushOptionsWidget class
 */
class KisToolLazyBrushOptionsWidget : public QWidget
{
    Q_OBJECT
private /* typedef */:
    typedef KisSwatchGroup::SwatchInfo SwatchInfoType;

public:
    KisToolLazyBrushOptionsWidget(KisCanvasResourceProvider *provider, QWidget *parent);
    ~KisToolLazyBrushOptionsWidget() override;

private Q_SLOTS:
    void entrySelected(QModelIndex index);
    void slotCurrentFgColorChanged(const KoColor &color);
    void slotCurrentNodeChanged(KisNodeSP node);
    void slotColorLabelsChanged();

    void slotMakeTransparent(bool value);
    void slotRemove();

    void slotUpdate();
    void slotSetAutoUpdates(bool value);
    void slotSetShowKeyStrokes(bool value);
    void slotSetShowOutput(bool value);

    void slotUseEdgeDetectionChanged(bool value);
    void slotEdgeDetectionSizeChanged(int value);
    void slotRadiusChanged(int value);
    void slotCleanUpChanged(int value);
    void slotLimitToDeviceChanged(bool value);


    void slotUpdateNodeProperties();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private /* methods */:
    static bool sortSwatchInfo(const SwatchInfoType &first, const SwatchInfoType &second);

private /* member variables */:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_TOOL_LAZY_BRUSH_OPTIONS_WIDGET_H */
