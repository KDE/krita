/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
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

#ifndef _KIS_COLOR_INPUT_H_
#define _KIS_COLOR_INPUT_H_

#include <QWidget>

class KoChannelInfo;
class KoColor;
class QWidget;
class KNumInput;
class KIntNumInput;
class KDoubleNumInput;
class QLabel;
class QHBoxLayout;

class KisColorInput : public QWidget
{
    Q_OBJECT
public:
    KisColorInput(QWidget* parent, const KoChannelInfo*, KoColor* color);
protected:
    void init();
    virtual KNumInput* createInput() = 0;
signals:
    void updated();
protected:
    const KoChannelInfo* m_channelInfo;
    KoColor* m_color;
    KNumInput* m_input;
    QLabel* m_label;
    QHBoxLayout *m_layout;
};

class KisIntegerColorInput : public KisColorInput
{
    Q_OBJECT
public:
    KisIntegerColorInput(QWidget* parent, const KoChannelInfo*, KoColor* color);
protected:
    virtual KNumInput* createInput();
public slots:
    void setValue(int);
    void update();
private:
    KIntNumInput* m_intNumInput;
};


class KisFloatColorInput : public KisColorInput
{
    Q_OBJECT
public:
    KisFloatColorInput(QWidget* parent, const KoChannelInfo*, KoColor* color);
protected:
    virtual KNumInput* createInput();
public slots:
    void setValue(double);
    void update();
private:
    KDoubleNumInput* m_dblNumInput;
};

#endif
