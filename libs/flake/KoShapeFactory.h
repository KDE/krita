/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#ifndef _KO_SHAPE_FACTORY_
#define _KO_SHAPE_FACTORY_

#include <QString>
#include <QWidget>
#include <QPixmap>
#include <QList>

#include <KoID.h>

#include <koffice_export.h>

class KoShape;
class KoProperties;

/**
 * Contains a KoProperties object that describes the settings of a
 * particular variant of a shape object, together with a name, a description
 * and a pixmap for use in the user interface.
 */
struct FLAKE_EXPORT KoShapeTemplate {

    QString name;
    QString description;
    QString tooltip;
    QPixmap pixmap;
    KoProperties * params;
};

/**
 * The shape factory can create a KoShape without needing a tool or a direct new().
 */
class FLAKE_EXPORT KoShapeFactory : public QObject {
    Q_OBJECT
public:

    /// Factory for shapes
    KoShapeFactory(const QString id, const QString name);
    virtual ~KoShapeFactory() {}

    virtual KoShape * createDefaultShape() = 0;
    virtual KoShape * createShape(KoProperties * params) const = 0;
    virtual KoShape * createShapeFromTemplate(KoShapeTemplate * shapeTemplate) const = 0;
    virtual QWidget * optionWidget() const = 0;

    const KoID id() const;
    const QString & shapeId() const;
    const QList<KoProperties*> templates() const { return m_templates; }
    const QString & toolTip() const;
    const QPixmap & icon() const;
    const QString & name() const;

protected:

    void addTemplate(KoProperties * params);
    void setToolTip(const QString & tooltip);
    void setIcon(const QPixmap & icon);
    void setOptionWidget(QWidget * widget);

private:

    QList<KoProperties*> m_templates;
    QString m_tooltip;
    QPixmap m_icon;
    const QString m_id, m_name;
};

#endif // _KO_SHAPE_FACTORY_
