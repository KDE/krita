/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef LIBKISUTILS_H
#define LIBKISUTILS_H

class Node;

#include <kis_types.h>

namespace LibKisUtils
{

QList<Node *> createNodeList(KisNodeList kisnodes, KisImageWSP image);

}

#endif // LIBKISUTILS_H
