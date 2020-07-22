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

#include <KoResource.h>
#include <QMetaType>
#include <kritaflake_export.h>

class KisSeExprScript;
typedef QSharedPointer<KisSeExprScript> KisSeExprScriptSP;

/**
 * KoResource container for SeExpr scripts.
 */
class KRITAFLAKE_EXPORT KisSeExprScript : public KoResource
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

    /**
     * Clones the given SeExpr script into a new instance.
     *
     * @param rhs the original SeExpr script to be copied onto this object
     */
    KisSeExprScript(KisSeExprScript *rhs);
    KisSeExprScript(const KisSeExprScript &rhs);

    ~KisSeExprScript();

public:
    /**
     * Load this resource.
     * @return true if loading the resource succeeded.
     */
    bool loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface) override;

    /**
     * Save this resource.
     * @return true if saving the resource succeeded.
     */
    bool saveToDevice(QIODevice *dev) const override;

    QPair<QString, QString> resourceType() const override;

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

    KoResourceSP clone() const override;

    bool isDirty() const;
    void setDirty(bool value);

private:
    struct Private;
    Private *const d;
};

Q_DECLARE_METATYPE(KisSeExprScript *)

#endif // KIS_SEEXPR_SCRIPT_H
