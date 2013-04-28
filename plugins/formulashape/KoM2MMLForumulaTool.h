/* This file is part of the KDE project
   Copyright (C) 2011 Cyrille Berger <cberger@cberger.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#ifndef KOM2MMLFORMULATOOL_H_
#define KOM2MMLFORMULATOOL_H_

#include <KoToolBase.h>
#include <KoToolFactoryBase.h>

class QComboBox;
class QLabel;
class KoFormulaShape;
class QLineEdit;

class KoM2MMLFormulaTool : public KoToolBase
{
    Q_OBJECT
public:
    explicit KoM2MMLFormulaTool(KoCanvasBase *canvas);
    
    virtual void activate(ToolActivation toolActivation, const QSet< KoShape* >& shapes);
    
    virtual void mouseReleaseEvent(KoPointerEvent* event);
    virtual void mousePressEvent(KoPointerEvent* event);
    virtual void mouseMoveEvent(KoPointerEvent* event);
    virtual void paint(QPainter& painter, const KoViewConverter& converter);
    virtual QWidget* createOptionWidget();
public slots:
    void textEdited();
private:
    void setMathML(const QString& mathml, const QString& mode);
private:
    QLineEdit* m_lineEdit;
    QLabel* m_errorLabel;
    KoFormulaShape* m_formulaShape;
    QString m_text;
    QComboBox* m_comboBox;
    QString m_mode;
};

class KoM2MMLFormulaToolFactory : public KoToolFactoryBase {
public:
    /// The constructor - reimplemented from KoToolFactoryBase
    explicit KoM2MMLFormulaToolFactory();

    /// The destructor - reimplemented from KoToolFactoryBase
    ~KoM2MMLFormulaToolFactory();

    /// @return an instance of KoFormulaTool
    KoToolBase* createTool( KoCanvasBase* canvas );
};

#endif
