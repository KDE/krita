/*
 *  Copyright (c) 2018 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISGAMUTMASKCHOOSER_H
#define KISGAMUTMASKCHOOSER_H

#include <QWidget>

class KoResourceItemChooser;
class KoResource;
class KoGamutMask;

class KisGamutMaskChooser : public QWidget
{
    Q_OBJECT
public:
    explicit KisGamutMaskChooser(QWidget *parent = nullptr);
    ~KisGamutMaskChooser() override;

    void setCurrentResource(KoResource* resource);

Q_SIGNALS:
    void sigGamutMaskSelected(KoGamutMask* mask);

private Q_SLOTS:
    void resourceSelected(KoResource* resource);

private:
    KoResourceItemChooser* m_itemChooser;
};

#endif // KISGAMUTMASKCHOOSER_H
