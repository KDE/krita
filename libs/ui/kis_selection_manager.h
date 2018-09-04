/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_SELECTION_MANAGER_
#define KIS_SELECTION_MANAGER_

#include <QObject>
#include <QList>
#include <QPointer>

#include <kis_image.h>
#include "KisView.h"
#include <KisSelectionTags.h>

#include <kritaui_export.h>

class KisActionManager;
class KisAction;
class QAction;
class KoViewConverter;
class KisDocument;

class KisViewManager;
class KisClipboard;
class KisNodeCommandsAdapter;
class KisView;

class KisSelectionFilter;
class KisSelectionDecoration;

/**
 * The selection manager is responsible selections
 * and the clipboard.
 */
class KRITAUI_EXPORT KisSelectionManager : public QObject
{

    Q_OBJECT
    Q_PROPERTY(bool displaySelection READ displaySelection NOTIFY displaySelectionChanged);
    Q_PROPERTY(bool havePixelsSelected READ havePixelsSelected NOTIFY currentSelectionChanged);
public:

    KisSelectionManager(KisViewManager * view);
    ~KisSelectionManager() override;

    void setup(KisActionManager* actionManager);

    void setView(QPointer<KisView>imageView);

public:
    /**
     * This function return if the selection should be displayed
     */
    bool displaySelection();

    bool showSelectionAsMask() const;

public Q_SLOTS:

    void updateGUI();
    void selectionChanged();
    void clipboardDataChanged();

    void cut();
    void copy();

    void cutSharp();
    void copySharp();

    void copyMerged();
    void paste();
    void pasteNew();
    void pasteAt();
    void cutToNewLayer();
    void selectAll();
    void deselect();
    void invert();
    void clear();
    void fillForegroundColor();
    void fillBackgroundColor();
    void fillPattern();
    void fillForegroundColorOpacity();
    void fillBackgroundColorOpacity();
    void fillPatternOpacity();
    void reselect();
    void editSelection();
    void convertToVectorSelection();
    void convertToRasterSelection();
    void convertShapesToVectorSelection();
    void convertToShape();
    
    void copySelectionToNewLayer();
    void toggleDisplaySelection();

    void shapeSelectionChanged();
    void imageResizeToSelection();
    void paintSelectedShapes();

    void slotToggleSelectionDecoration();

    void slotStrokeSelection();

    void selectOpaqueOnNode(KisNodeSP node, SelectionAction action);

Q_SIGNALS:
    void currentSelectionChanged();
    void signalUpdateGUI();
    void displaySelectionChanged();
    void strokeSelected();

public:
    bool havePixelsSelected();
    bool havePixelsInClipboard();
    bool haveShapesSelected();
    bool haveShapesInClipboard();

    /// Checks if the current selection is editable and has some pixels selected in the pixel selection
    bool haveAnySelectionWithPixels();
    bool haveShapeSelectionWithShapes();
    bool haveRasterSelectionWithPixels();

private:
    void fill(const KoColor& color, bool fillWithPattern, const QString& transactionText);
    void updateStatusBar();

    KisViewManager * m_view;
    KisDocument * m_doc;
    QPointer<KisView>m_imageView;
    KisClipboard * m_clipboard;

    KisNodeCommandsAdapter* m_adapter;

    KisAction *m_copy;
    KisAction *m_copyMerged;
    KisAction *m_cut;
    KisAction *m_paste;
    KisAction *m_pasteAt;
    KisAction *m_pasteNew;
    KisAction *m_cutToNewLayer;
    KisAction *m_selectAll;
    KisAction *m_deselect;
    KisAction *m_clear;
    KisAction *m_reselect;
    KisAction *m_invert;
    KisAction *m_copyToNewLayer;
    KisAction *m_fillForegroundColor;
    KisAction *m_fillBackgroundColor;
    KisAction *m_fillPattern;
    KisAction *m_fillForegroundColorOpacity;
    KisAction *m_fillBackgroundColorOpacity;
    KisAction *m_fillPatternOpacity;
    KisAction *m_imageResizeToSelection;
    KisAction *m_strokeShapes;
    KisAction *m_toggleDisplaySelection;
    KisAction *m_toggleSelectionOverlayMode;
    KisAction *m_strokeSelected;


    QList<QAction*> m_pluginActions;
    QPointer<KisSelectionDecoration> m_selectionDecoration;

};

#endif // KIS_SELECTION_MANAGER_
