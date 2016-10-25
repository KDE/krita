/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef LIBKIS_DOCUMENT_H
#define LIBKIS_DOCUMENT_H

#include <QObject>

#include "kritalibkis_export.h"
#include "libkis.h"

class KisDocument;

/**
 * Document
 */
class KRITALIBKIS_EXPORT Document : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Document)

    Q_PROPERTY(Node* ActiveNode READ activeNode WRITE setActiveNode)
    Q_PROPERTY(ColorDepth* ColorDepth READ colorDepth WRITE setColorDepth)
    Q_PROPERTY(ColorManager* ColorManager READ colorManager WRITE setColorManager)
    Q_PROPERTY(ColorModel* ColorModel READ colorModel WRITE setColorModel)
    Q_PROPERTY(ColorProfile* ColorProfile READ colorProfile WRITE setColorProfile)
    Q_PROPERTY(InfoObject* DocumentInfo READ documentInfo WRITE setDocumentInfo)
    Q_PROPERTY(QString FileName READ fileName WRITE setFileName)
    Q_PROPERTY(int Height READ height WRITE setHeight)
    Q_PROPERTY(InfoObject* MetaData READ metaData WRITE setMetaData)
    Q_PROPERTY(QString Name READ name WRITE setName)
    Q_PROPERTY(int Resolution READ resolution WRITE setResolution)
    Q_PROPERTY(Node* RootNode READ rootNode WRITE setRootNode)
    Q_PROPERTY(Selection* Selection READ selection WRITE setSelection)
    Q_PROPERTY(int Width READ width WRITE setWidth)
    Q_PROPERTY(QByteArray PixelData READ pixelData WRITE setPixelData)

public:
    explicit Document(KisDocument *document, QObject *parent = 0);
    virtual ~Document();

    Node* activeNode() const;
    void setActiveNode(Node* value);

    ColorDepth* colorDepth() const;
    void setColorDepth(ColorDepth* value);

    ColorManager* colorManager() const;
    void setColorManager(ColorManager* value);

    ColorModel* colorModel() const;
    void setColorModel(ColorModel* value);

    ColorProfile* colorProfile() const;
    void setColorProfile(ColorProfile* value);

    InfoObject* documentInfo() const;
    void setDocumentInfo(InfoObject* value);

    QString fileName() const;
    void setFileName(QString value);

    int height() const;
    void setHeight(int value);

    InfoObject* metaData() const;
    void setMetaData(InfoObject* value);

    QString name() const;
    void setName(QString value);

    int resolution() const;
    void setResolution(int value);

    Node* rootNode() const;
    void setRootNode(Node* value);

    Selection* selection() const;
    void setSelection(Selection* value);

    int width() const;
    void setWidth(int value);

    QByteArray pixelData() const;
    void setPixelData(QByteArray value);

public Q_SLOTS:

    Document * clone();

    bool close();

    bool convert(const QString &colorModel, const ColorProfile *profile);

    void crop(int x, int y, int w, int h);

    bool Export(const InfoObject &exportConfiguration);

    void Flatten();

    void ResizeImage(int w, int h);

    bool Save(const QString &url);

    bool SaveAs(const QString &url);

    void OpenView();

    Node* CreateNode(const QString &name, const QString &nodeType);

private:
    struct Private;
    Private *const d;

};

#endif // LIBKIS_DOCUMENT_H
