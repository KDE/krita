/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_ACYCLIC_SIGNAL_CONNECTOR_H
#define __KIS_ACYCLIC_SIGNAL_CONNECTOR_H

#include <QObject>
#include "krita_export.h"


/**
 * A special class for connecting UI elements to manager classes.
 * It allows to avoid direct calling blockSignals() for the sender UI
 * element all the time. This is the most important when the measured
 * value can be changed not only by the user through the UI, but also
 * by the manager according to some internal rules.
 *
 * Example:
 *
 * Suppose we have the following connections:
 *
 * 1) QDoubleSpinBox::valueChanged(double) -> Manager::slotSetValue(double)
 * 2) Manager::valueChanged(double) -> QDoubleSpinBox::setValue(double)
 *
 * Now if the manager decides to change/correct the value, the spinbox
 * will go into an infinite loop.
 *
 * See an example in KisToolCropConfigWidget.
 */

class KRITAIMAGE_EXPORT KisAcyclicSignalConnector : public QObject
{
    Q_OBJECT
public:
    KisAcyclicSignalConnector(QObject *parent);

    void connectForwardDouble(QObject *sender, const char *signal,
                              QObject *receiver, const char *method);

    void connectBackwardDouble(QObject *sender, const char *signal,
                               QObject *receiver, const char *method);

    void connectForwardInt(QObject *sender, const char *signal,
                           QObject *receiver, const char *method);

    void connectBackwardInt(QObject *sender, const char *signal,
                            QObject *receiver, const char *method);

    void connectForwardBool(QObject *sender, const char *signal,
                            QObject *receiver, const char *method);

    void connectBackwardBool(QObject *sender, const char *signal,
                             QObject *receiver, const char *method);

    void connectForwardVoid(QObject *sender, const char *signal,
                            QObject *receiver, const char *method);

    void connectBackwardVoid(QObject *sender, const char *signal,
                             QObject *receiver, const char *method);

private slots:
    void forwardSlotDouble(double value);
    void backwardSlotDouble(double value);

    void forwardSlotInt(int value);
    void backwardSlotInt(int value);

    void forwardSlotBool(bool value);
    void backwardSlotBool(bool value);

    void forwardSlotVoid();
    void backwardSlotVoid();

signals:
    void forwardSignalDouble(double value);
    void backwardSignalDouble(double value);

    void forwardSignalInt(int value);
    void backwardSignalInt(int value);

    void forwardSignalBool(bool value);
    void backwardSignalBool(bool value);

    void forwardSignalVoid();
    void backwardSignalVoid();

private:
    int m_signalsBlocked;
};

#endif /* __KIS_ACYCLIC_SIGNAL_CONNECTOR_H */
