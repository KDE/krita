/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOABSTRACTCANVASRESOURCEINTERFACE_H
#define KOABSTRACTCANVASRESOURCEINTERFACE_H

#include <QObject>
#include <QScopedPointer>
#include <QSharedPointer>
#include "kritaflake_export.h"

class QVariant;

/**
 * \class KoAbstractCanvasResourceInterface
 *
 * Defines an abstract resource that is stored outside the resource manager
 */
class KRITAFLAKE_EXPORT KoAbstractCanvasResourceInterface : public QObject
{
    Q_OBJECT
public:
    KoAbstractCanvasResourceInterface(int key, const QString debugTag = QString());

    /**
     * Return the current value of the resource
     */
    virtual QVariant value() const = 0;

    /**
     * @brief set the value of the current resource
     */
    virtual void setValue(const QVariant value) = 0;

    /**
     * The key corresponding to the resource
     */
    int key() const;

Q_SIGNALS:
    /**
     * The signal is emitted when the resource is changed outside
     * the setValue() call by some external entity
     */
    void sigResourceChangedExternal(int key, const QVariant &value);

private:
    int m_key = -1;
    QString m_debugTag;
};

typedef QSharedPointer<KoAbstractCanvasResourceInterface> KoAbstractCanvasResourceInterfaceSP;

#endif // KOABSTRACTCANVASRESOURCEINTERFACE_H
