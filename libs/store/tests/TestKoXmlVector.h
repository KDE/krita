/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2015 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef TESTKOXMLVECTOR_H
#define TESTKOXMLVECTOR_H

// Qt
#include <QObject>

class TestKoXmlVector : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void simpleConstructor();
    void writeAndRead_data();
    void writeAndRead();
};

#endif
