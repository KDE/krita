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

#include <kis_image.h>

#include "kis_doc2.h"

#include <krita_export.h>

class KAction;
class KToggleAction;
class KActionCollection;
class KoViewConverter;

class KisView2;
class KisDoc;
class KisClipboard;
class KisNodeCommandsAdapter;

class KisSelectionFilter;

/**
 * The selection manager is responsible selections
 * and the clipboard.
 */
class KRITAUI_EXPORT KisSelectionManager : public QObject
{

    Q_OBJECT

public:

    KisSelectionManager(KisView2 * view, KisDoc2 * doc);
    virtual ~KisSelectionManager();

    void setup(KActionCollection * collection);

    void addSelectionAction(QAction * action);

public:
    /**
     * This function return if the selection should be displayed
     */
    bool displaySelection();

public slots:

    void updateGUI();
    void selectionChanged();
    void clipboardDataChanged();

    void cut();
    void copy();
    void copyMerged();
    void paste();
    void pasteNew();
    void pasteAt();
    void cutToNewLayer();
    void selectAll();
    void deselect();
    void clear();
    void fillForegroundColor();
    void fillBackgroundColor();
    void fillPattern();
    void reselect();
    void invert();
    void smooth();
    void copySelectionToNewLayer();
    void toggleDisplaySelection();

    void shapeSelectionChanged();
    void imageResizeToSelection();

signals:
    void currentSelectionChanged();
    void signalUpdateGUI();

public:
    bool havePixelsSelected();
    bool havePixelsInClipboard();
    bool haveShapesSelected();
    bool haveShapesInClipboard();

    /// Checks if the current selection is editabl and has some pixels selected in the pixel selection
    bool haveEditablePixelSelectionWithPixels();

    void grow(qint32 xradius, qint32 yradius);
    void shrink(qint32 xradius, qint32 yradius, bool edge_lock);
    void border(qint32 xradius, qint32 yradius);
    void feather(qint32 radius);
    // the following functions are needed for the siox tool
    // they might be also useful on its own
    void erode();
    void dilate();

    void paint(QPainter& gc, const KoViewConverter &converter);

private:
    void fill(const KoColor& color, bool fillWithPattern, const QString& transactionText);
    void updateStatusBar();

    void copyFromDevice(KisPaintDeviceSP device);
    void applySelectionFilter(KisSelectionFilter *filter);

    KisView2 * m_view;
    KisDoc2 * m_doc;

    KisClipboard * m_clipboard;

    KisNodeCommandsAdapter* m_adapter;

    KAction *m_copy;
    KAction *m_copyMerged;
    KAction *m_cut;
    KAction *m_paste;
    KAction *m_pasteAt;
    KAction *m_pasteNew;
    KAction *m_cutToNewLayer;
    KAction *m_selectAll;
    KAction *m_deselect;
    KAction *m_clear;
    KAction *m_reselect;
    KAction *m_invert;
    KAction *m_copyToNewLayer;
    KAction *m_smooth;
    KAction *m_load;
    KAction *m_save;
    KAction *m_fillForegroundColor;
    KAction *m_fillBackgroundColor;
    KAction *m_fillPattern;
    KAction *m_imageResizeToSelection;
    KToggleAction *m_toggleDisplaySelection;

    QList<QAction*> m_pluginActions;

};

#endif // KIS_SELECTION_MANAGER_
