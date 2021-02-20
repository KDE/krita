/*
  SPDX-FileCopyrightText: 2006 Gábor Lehel <illissius@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIS_DOCUMENT_SECTION_TOOLTIP_H
#define KIS_DOCUMENT_SECTION_TOOLTIP_H

#include "KoItemToolTip.h"

class KisNodeModel;

/**
 * A default tooltip for a NodeView that shows a thumbnail
 * image and the list of properties associated with a node.
 */
class NodeToolTip: public KoItemToolTip
{
    Q_OBJECT

public:
    NodeToolTip();
    ~NodeToolTip() override;

protected:
    QTextDocument *createDocument(const QModelIndex &index) override;

private:
    typedef KisNodeModel Model;
};

#endif
