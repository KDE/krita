/*
 *  SPDX-FileCopyrightText: 2003 Patrick Julien <freak@codepimps.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_UNDO_ADAPTER_H_
#define KIS_UNDO_ADAPTER_H_

#include <QObject>

#include <kritaimage_export.h>
#include <kis_undo_store.h>


class KRITAIMAGE_EXPORT KisUndoAdapter : public QObject
{
    Q_OBJECT

public:
    KisUndoAdapter(KisUndoStore *undoStore, QObject *parent = 0);
    ~KisUndoAdapter() override;

public:
    void emitSelectionChanged();

    virtual const KUndo2Command* presentCommand() = 0;
    virtual void undoLastCommand() = 0;
    virtual void addCommand(KUndo2Command *cmd) = 0;
    virtual void beginMacro(const KUndo2MagicString& macroName) = 0;
    virtual void endMacro() = 0;

    inline void setUndoStore(KisUndoStore *undoStore) {
        m_undoStore = undoStore;
    }

Q_SIGNALS:
    void selectionChanged();

protected:
    inline KisUndoStore* undoStore() {
        return m_undoStore;
    }

private:
    Q_DISABLE_COPY(KisUndoAdapter)
    KisUndoStore *m_undoStore;
};


#endif // KIS_UNDO_ADAPTER_H_

