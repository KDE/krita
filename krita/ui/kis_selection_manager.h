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

#include "qobject.h"
#include "q3ptrlist.h"

#include "kis_image.h"
#include <krita_export.h>

class KAction;
class KToggleAction;
class KisView;
class KisDoc;
class KisClipboard;

/**
 * The selection manager is responsible selections
 * and the clipboard.
 */
class KRITAUI_EXPORT KisSelectionManager : public QObject {

    Q_OBJECT

public:

    KisSelectionManager(KisView * parent, KisDoc * doc);
    virtual ~KisSelectionManager();

    void setup(KActionCollection * collection);

    void addSelectionAction(KAction * action);

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
    // they might be also usefull on its own
    void erode();
    void dilate();

private:
    void fill(const KisColor& color, bool fillWithPattern, const QString& transactionText);

    void computeBorder (qint32  *circ, qint32  xradius, qint32  yradius);
    inline void rotatePointers (quint8  **p, quint32 n);
    void computeTransition (quint8* transition, quint8** buf, qint32 width);

    KisView * m_parent;
    KisDoc * m_doc;

    KisClipboard * m_clipboard;

    KAction *m_copy;
    KAction *m_cut;
    KAction *m_paste;
    KAction *m_pasteNew;
    KAction *m_cutToNewLayer;
    KAction *m_selectAll;
    KAction *m_deselect;
    KAction *m_clear;
    KAction *m_reselect;
    KAction *m_invert;
    KAction *m_toNewLayer;
    KAction *m_feather;
    KAction *m_border;
    KAction *m_expand;
    KAction *m_smooth;
    KAction *m_contract;
    KAction *m_similar;
    KAction *m_transform;
    KAction *m_load;
    KAction *m_save;
    KAction *m_fillForegroundColor;
    KAction *m_fillBackgroundColor;
    KAction *m_fillPattern;
    KToggleAction *m_toggleDisplaySelection;

    Q3PtrList<KAction> m_pluginActions;

};

#endif // KIS_SELECTION_MANAGER_
