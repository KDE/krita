/*
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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
#include <QtCore/QtCore>
#include <QtXml/QtXml>

/*
Note that we store the doubles as 8 bytes in a cdata section. We really need to do that
because going to and from and back to a text representation of doubles loses precision.
 */

double **allocateMatrix(int m, int n)
{
    double **matrix;
    matrix = new double*[m];
    for (int i = 0; i < m; i++)
        matrix[i] = new double[n];
    return matrix;
}

void writeTransformations(QDomDocument & doc, QDomElement & transformations, double ** T, int numWavelengths)
{
    for (int i = 0; i < numWavelengths; i++) {
        for (int j = 0; j < 3; j++) {
            QDomElement transformation = doc.createElement("transformation");
            transformation.setAttribute("j", j);
            transformation.setAttribute("i", i);
            transformation.setAttribute("value", T[j][i]);
            //QByteArray bytes((char*)&T[j][i], 8);
            //QDomCDATASection cdata = doc.createCDATASection(bytes);
            //transformation.appendChild(cdata);
            transformations.appendChild(transformation);
        }
    }
}

void writePrimary(QDomDocument & doc, QDomElement & node, double * primary, int numWavelengths)
{
    for (int i = 0; i < numWavelengths; ++i) {
        QDomElement wavelength = doc.createElement("wavelength");
        wavelength.setAttribute("value", primary[i]);
        //QByteArray bytes((char*)(primary + i), 8);
        //QDomCDATASection cdata = doc.createCDATASection(bytes);
        //wavelength.appendChild(cdata);
        node.appendChild(wavelength);
    }
}

int main(int c, char **v)
{
    QCoreApplication app(c, v);

    if (sizeof(double) != 8) {
        QTextStream(stdout) << "double is the wrong size " << sizeof(double) << endl;
        return 1;
    }


    if (c < 2) {
        QTextStream(stdout) << "Usage; matlab2xml input > output" << endl;
        return 1;
    }

    QFile file(app.arguments()[1]);
    if (! file.open(QIODevice::ReadOnly)) {
        QTextStream(stderr) << "Could not open file for reading; " << app.arguments()[1] << endl;
        return 1;
    }

    /*
    fwrite(fid,[profile.name 0]);
    fwrite(fid,profile.N,'char*1');
    fwrite(fid,profile.T,'double');
    fwrite(fid,profile.P,'double');
    fwrite(fid,profile.coeffs,'char*1');
    fwrite(fid,profile.Xmin,'double');

    Data is saved cols-first.

    T is a matrix with 3 rows and N cols

    P is a matrix with 3 cols and N rows

    Xmin is a vector of N*coeffs doubles
    */
    int m_wl = -1;
    double **m_T = 0;
    double *m_red = 0;
    double *m_green = 0;
    double *m_blue = 0;

    int nc = 0;
    double *coeffs = 0;

    QString m_illuminant = "";

    // Loading code copied from kis_illumnint_profile.cpp
    QDataStream data(&file);

    // Loop until we encounter a 0
    {
        qint8 letter;
        while (true) {
            data >> letter;
            if (!letter)
                break;
            m_illuminant += (char)letter;
        }
    }
    {
        qint8 tmp;
        data >> tmp;
        m_wl = (int)tmp;
        m_T = allocateMatrix(3, m_wl);
        for (int i = 0; i < m_wl; i++)
            for (int j = 0; j < 3; j++)
                data.readRawData((char*)&m_T[j][i], 8);
    }
    {
        m_red   = new double[m_wl];
        m_green = new double[m_wl];
        m_blue  = new double[m_wl];
        for (int i = 0; i < m_wl; i++) {
            data.readRawData((char*)&m_red[i], 8);
        }
        for (int i = 0; i < m_wl; i++) {
            data.readRawData((char*)&m_green[i], 8);
        }
        for (int i = 0; i < m_wl; i++) {
            data.readRawData((char*)&m_blue[i], 8);
        }
    }
    {
        qint8 tmp;
        data >> tmp;
        nc = (int)tmp;
        coeffs = new double[nc*m_wl];
        for (quint8 i = 0; i < nc*m_wl; i++)
            data.readRawData((char*)&coeffs[i], 8);
    }

    // Create a dom document & save it
    QDomDocument doc("illuminant");
    QDomElement root = doc.createElement("illuminant");
    doc.appendChild(root);

    root.setAttribute("version", 1);
    root.setAttribute("name", m_illuminant);
    root.setAttribute("wavelengths", m_wl);

    QDomElement transformations = doc.createElement("transformations");
    root.appendChild(transformations);
    writeTransformations(doc, transformations, m_T, m_wl);

    QDomElement primaries = doc.createElement("primaries");
    root.appendChild(primaries);

    QDomElement red = doc.createElement("red");
    primaries.appendChild(red);
    writePrimary(doc, red, m_red, m_wl);

    QDomElement green = doc.createElement("green");
    primaries.appendChild(green);
    writePrimary(doc, green, m_green, m_wl);

    QDomElement blue = doc.createElement("blue");
    primaries.appendChild(blue);
    writePrimary(doc, blue, m_blue, m_wl);

    QDomElement coefficients = doc.createElement("X");
    coefficients.setAttribute("nc", nc);
    root.appendChild(coefficients);

    for (int i = 0; i < m_wl * nc; ++i) {
        QDomElement coeff = doc.createElement("coefficient");
        coeff.setAttribute("value", coeffs[i]);
        //QByteArray bytes((char*)(coeffs + i), 8);
        //QDomCDATASection cdata = doc.createCDATASection(bytes);
        coefficients.appendChild(coeff);
        //coeff.appendChild(cdata);
    }

    QTextStream(stdout) << doc.toString();


}
