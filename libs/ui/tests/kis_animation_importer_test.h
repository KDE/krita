/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_ANIMATION_IMPORTER_TEST_H
#define KIS_ANIMATION_IMPORTER_TEST_H

#include <simpletest.h>

class KisAnimationImporterTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void testImport();
};

#endif
