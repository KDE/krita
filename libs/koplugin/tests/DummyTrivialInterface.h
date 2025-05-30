/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef DUMMY_TRIVIAL_INTERFACE_H
#define DUMMY_TRIVIAL_INTERFACE_H

#include <QObject>
#include <QString>

#include <dummytrivialinterface_export.h>

class DUMMYTRIVIALINTERFACE_EXPORT DummyTrivialInterface : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    ~DummyTrivialInterface() override;
    virtual QString name() const = 0;
    virtual int version() const = 0;
};

#endif // DUMMY_TRIVIAL_INTERFACE_H