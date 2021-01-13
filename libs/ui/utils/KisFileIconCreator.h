/*
 *  SPDX-FileCopyrightText: 2020 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_FILE_ICON_CREATOR_H
#define KIS_FILE_ICON_CREATOR_H

#include <QScopedPointer>
#include <QIcon>

#include <KisPreviewFileDialog.h>

#include "kritaui_export.h"

/**
 * @brief The KisFileIconCreator class creates a thumbnail from a file on disk
 *
 * On Welcome Page and possibly other places there might be a need to show the user
 * a thumbnail of a file. This class tries to open a file and create a thumbnail out of it.
 *
 * In theory creating the object is not needed, so if you, dear future reader, want to convert
 * the function inside to a static one, go ahead.
 *
 */
class KRITAUI_EXPORT KisFileIconCreator : public KisAbstractFileIconCreator
{

public:
    /**
     * @brief createFileIcon creates an icon from the file on disk
     * @param path path to the file
     * @param icon created icon
     * @param devicePixelRatioF a result from devicePixelRatioF() called in a widget
     * @param iconSize size of the icon
     * @return true if icon was created successfully, false if not (for example the file doesn't exist)
     */
    bool createFileIcon(QString path, QIcon &icon, qreal devicePixelRatioF, QSize iconSize, bool dontUpsize = false) override;
};

#endif // KIS_FILE_ICON_CREATOR_H
