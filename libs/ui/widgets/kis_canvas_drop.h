/*
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_CANVAS_DROP_H
#define KIS_CANVAS_DROP_H

#include <QMenu>
#include <QMimeData>
#include <QPoint>

class KisCanvasDrop : private QMenu
{
    Q_OBJECT

public:
    enum Action {
        NONE = 0,

        INSERT_AS_NEW_LAYER,
        INSERT_AS_NEW_FILE_LAYER,
        OPEN_IN_NEW_DOCUMENT,
        INSERT_AS_REFERENCE_IMAGE,

        INSERT_MANY_LAYERS,
        INSERT_MANY_FILE_LAYERS,
        OPEN_MANY_DOCUMENTS,
        INSERT_AS_REFERENCE_IMAGES,
    };

    KisCanvasDrop(QWidget *parent = nullptr);

    Action dropAs(const QMimeData &data, QPoint pos);

private:
    class Private;
    Private *const d;
};

#endif // KIS_CANVAS_DROP_H
