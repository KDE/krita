/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_COLOR_HISTORY_H
#define KIS_COLOR_HISTORY_H

#include "kis_color_patches.h"

class QToolButton;
class KisCanvasResourceProvider;
class KisColorHistoryNotifier;

class KisColorHistory : public KisColorPatches
{
    Q_OBJECT
public:
    explicit KisColorHistory(QWidget *parent = 0);
    void setCanvas(KisCanvas2 *canvas) override;
    void unsetCanvas() override;

protected:
    KisColorSelectorBase* createPopup() const override;

public Q_SLOTS:
    void addColorToHistory(const KoColor& color);

    void clearColorHistory();

    void updateUserSettings();

    // Receive notification that the color history changed in some selector
    void colorHistoryChanged(const QList<KoColor> &history);

private:
    // Get reference to the relevant color history, either from resource provider or document.
    QList<KoColor> colorHistory();

    // Write the changed color history where it is stored, depending on the settings.
    void updateColorHistory(const QList<KoColor> &history);

    bool m_history_per_document = false;

    QToolButton* m_clearButton;
    KisDocument *m_document; // Color history is now stored in the document
    KisCanvasResourceProvider  *m_resourceProvider; // to disconnect...
};

// Singleton instance to update color history when there are multiple windows that must have it in sync.
class KisColorHistoryNotifier : public QObject
{
Q_OBJECT

public:
    void notifyColorHistoryChanged(const QList<KoColor> &history);


Q_SIGNALS:

    void colorHistoryChanged(const QList<KoColor> &history);
};


#endif // KIS_COLOR_HISTORY_H
