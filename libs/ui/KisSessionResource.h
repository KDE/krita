/*
 *  copyright (c) 2018 jouni pentikäinen <joupent@gmail.com>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 51 franklin street, fifth floor, boston, ma 02110-1301, usa.
 */

#ifndef KISSESSIONRESOURCE_H
#define KISSESSIONRESOURCE_H

#include "KisWindowLayoutResource.h"

class KisSessionResource : public KisWindowLayoutResource
{
public:
    KisSessionResource(const QString &filename);
    ~KisSessionResource();

    void storeCurrentWindows();
    void restore();

    QString defaultFileExtension() const override;

protected:
    void saveXml(QDomDocument &doc, QDomElement &root) const override;

    void loadXml(const QDomElement &root) const override;

private:
    struct Private;
    QScopedPointer<Private> d;
};


#endif