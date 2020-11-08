/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_GRADIENT_CHOOSER_H_
#define KIS_GRADIENT_CHOOSER_H_

#include <KoDialog.h>
#include <KoColor.h>

#include <QFrame>
#include <QToolButton>
#include <kritaui_export.h>
#include <KoResource.h>
#include <KoAbstractGradient.h>
#include <KoStopGradient.h>
#include <KoSegmentGradient.h>
#include <KoCanvasResourcesInterface.h>

class KisViewManager;
class QLabel;
class QPushButton;

class KisResourceItemChooser;
class KisAutogradientEditor;
class KoResource;

class KisCustomGradientDialog : public KoDialog
{

    Q_OBJECT

public:

    KisCustomGradientDialog(KoAbstractGradientSP gradient, QWidget *parent, const char *name, KoCanvasResourcesInterfaceSP canvasResourcesInterface);

private:

    QWidget * m_page;

};

class KRITAUI_EXPORT KisGradientChooser : public QFrame
{

    Q_OBJECT

public:
    KisGradientChooser(QWidget *parent = 0, const char *name = 0);
    ~KisGradientChooser() override;

    void setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface);
    KoCanvasResourcesInterfaceSP canvasResourcesInterface() const;

    /// Gets the currently selected resource
    /// @returns the selected resource, 0 is no resource is selected
    KoResourceSP currentResource();
    void setCurrentResource(KoResourceSP resource);

    void setCurrentItem(int row);

Q_SIGNALS:
    /// Emitted when a resource was selected
    void resourceSelected(KoResourceSP resource);

public Q_SLOTS:
    void slotUpdateIcons();

private Q_SLOTS:
    virtual void update(KoResourceSP resource);
    void addStopGradient();
    void addSegmentedGradient();
    void editGradient();

private:
      void addGradient(KoAbstractGradientSP gradient, bool editGradient = false);
private:
    QLabel *m_lbName;
    KisResourceItemChooser * m_itemChooser;

    QToolButton* m_addGradient;
    QPushButton* m_editGradient;

    KoCanvasResourcesInterfaceSP m_canvasResourcesInterface;
};

#endif // KIS_GRADIENT_CHOOSER_H_

