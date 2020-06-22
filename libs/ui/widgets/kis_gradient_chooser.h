/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_GRADIENT_CHOOSER_H_
#define KIS_GRADIENT_CHOOSER_H_

#include <KoDialog.h>

#include <QFrame>
#include <QToolButton>
#include <kritaui_export.h>
#include <KoResource.h>
#include <KoAbstractGradient.h>
#include <KoStopGradient.h>
#include <KoSegmentGradient.h>

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

    KisCustomGradientDialog(KoAbstractGradientSP gradient, QWidget *parent, const char *name);

private:

    QWidget * m_page;

};

class KRITAUI_EXPORT KisGradientChooser : public QFrame
{

    Q_OBJECT

public:
    KisGradientChooser(QWidget *parent = 0, const char *name = 0);
    ~KisGradientChooser() override;

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
};

#endif // KIS_GRADIENT_CHOOSER_H_

