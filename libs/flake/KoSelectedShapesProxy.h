/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOSELECTEDSHAPESPROXY_H
#define KOSELECTEDSHAPESPROXY_H

#include <QObject>
#include "kritaflake_export.h"

class KoSelection;
class KoShapeLayer;

/**
 * @brief The KoSelectedShapesProxy class is a special interface of KoCanvasBase to
 * have a stable connection to shape selection signals in an environment when the
 * active shape manager can switch (e.g. when shape layers are switched in Krita)
 */

class KRITAFLAKE_EXPORT KoSelectedShapesProxy : public QObject
{
    Q_OBJECT
public:
    explicit KoSelectedShapesProxy(QObject *parent = 0);

    /**
     * Returns a pointer to a currently active shape selection. Don't connect to the
     * selection, unless you really know what you are doing. Use the signals provided
     * by KoSelectedShapesProxy itself. They are guaranteed to be valid all the time.
     */
    virtual KoSelection *selection() = 0;


    /**
      * @brief The shape wants to edited. This is used when a shape is passed
      * between two different tools. This notifies the new tool that it needs
      * to enter some extra edit mode.
      *
      */
    bool isRequestingToBeEdited();
    void setRequestingToBeEdited(bool value);

Q_SIGNALS:

    // forwards a corresponding signal of KoShapeManager
    void selectionChanged();

    // forwards a corresponding signal of KoShapeManager
    void selectionContentChanged();

    // forwards a corresponding signal of KoSelection
    void currentLayerChanged(const KoShapeLayer *layer);

private:
    bool m_isRequestingEditing = false;
};

#endif // KOSELECTEDSHAPESPROXY_H
