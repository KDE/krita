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

class KisView2;
class KisDoc;
class KisClipboard;

/**
 * The selection manager is responsible selections
 * and the clipboard.
 */
class KRITAUI_EXPORT KisSelectionManager : public QObject {

    Q_OBJECT

public:

    KisSelectionManager(KisView2 * parent, KisDoc2 * doc);
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
    void imgSelectionChanged(KisImageSP img);
    void clipboardDataChanged();

    void cut();
    void copy();
    KisLayerSP paste();
    void pasteNew();
    void cutToNewLayer();
    void selectAll();
    void deselect();
    void clear();
    void fillForegroundColor();
    void fillBackgroundColor();
    void fillPattern();
    void reselect();
    void invert();
    void copySelectionToNewLayer();
    void feather();
    void border();
    void expand();
    void contract();
    void smooth();
    void similar();
    void transform();
    void load();
    void save();
    void toggleDisplaySelection();

public:
    void grow (qint32 xradius, qint32 yradius);
    void shrink (qint32 xradius, qint32 yradius, bool edge_lock);
    void border(qint32 xradius, qint32 yradius);
    // the following functions are needed for the siox tool
    // they might be also useful on its own
    void erode();
    void dilate();

private:
    void fill(const KoColor& color, bool fillWithPattern, const QString& transactionText);
    void updateStatusBar();

    void computeBorder (qint32  *circ, qint32  xradius, qint32  yradius);
    inline void rotatePointers (quint8  **p, quint32 n);
    void computeTransition (quint8* transition, quint8** buf, qint32 width);

    KisView2 * m_parent;
    KisDoc2 * m_doc;

    KisClipboard * m_clipboard;

    QAction *m_copy;
    QAction *m_cut;
    QAction *m_paste;
    QAction *m_pasteNew;
    QAction *m_cutToNewLayer;
    QAction *m_selectAll;
    QAction *m_deselect;
    QAction *m_clear;
    QAction *m_reselect;
    QAction *m_invert;
    QAction *m_toNewLayer;
    QAction *m_feather;
    QAction *m_border;
    QAction *m_expand;
    QAction *m_smooth;
    QAction *m_contract;
    QAction *m_similar;
    QAction *m_transform;
    QAction *m_load;
    QAction *m_save;
    QAction *m_fillForegroundColor;
    QAction *m_fillBackgroundColor;
    QAction *m_fillPattern;
    KToggleAction *m_toggleDisplaySelection;

    QList<QAction*> m_pluginActions;

};

#endif // KIS_SELECTION_MANAGER_
