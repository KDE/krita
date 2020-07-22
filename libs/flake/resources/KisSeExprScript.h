/*
 * This file is part of Krita
 *
 * Copyright (c) 2020 L. E. Segovia <amy@amyspark.me>
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

#ifndef KIS_SEEXPR_SCRIPT_H
#define KIS_SEEXPR_SCRIPT_H

#include <QMetaType>
#include <kis_shared.h>
#include <kis_shared_ptr.h>
#include <kritaflake_export.h>
#include <resources/KoResource.h>

class KisSeExprScript;

typedef KisSharedPtr<KisSeExprScript> KisSeExprScriptSP;

/**
 * KoResource container for SeExpr scripts.
 */
class KRITAFLAKE_EXPORT KisSeExprScript : public KoResource, public KisShared
{
public:
    /**
     * Creates a new KisSeExprScript object using @p filename.  No file is opened
     * in the constructor, you have to call load.
     *
     * @param filename the file name to save and load from.
     */
    KisSeExprScript(const QString &filename);

    /**
     * Creates a new SeExpr script resource with the given @p image thumbnail,
     * @p script, @p name, @p and folder name.
     *
     * @param image the thumbnail of the texture this script will render
     * @param script the actual script's contents
     * @param name the name of the script
     * @param folderName the folder name
     */
    KisSeExprScript(const QImage &image, const QString &script, const QString &name, const QString &folderName);

    ~KisSeExprScript();

public:
    /**
     * Load this resource.
     * @return true if loading the resource succeeded.
     */
    bool load() override;
    bool loadFromDevice(QIODevice *dev) override;

    /**
     * Save this resource.
     * @return true if saving the resource succeeded.
     */
    bool save() override;
    bool saveToDevice(QIODevice *dev) const override;

    QString defaultFileExtension() const override;

    /**
     * @brief script the actual script
     * @return a valid SeExpr script. It is guaranteed to be in UTF-8.
     */
    QString script() const;

    /**
     * @brief set SeExpr expression script
     */
    void setScript(const QString &script);

    KisSeExprScript *clone() const;

    bool isDirty() const;
    void setDirty(bool value);

private:
    struct Private;
    Private *const d;
};

Q_DECLARE_METATYPE(KisSeExprScript *)

#endif // KIS_SEEXPR_SCRIPT_H
