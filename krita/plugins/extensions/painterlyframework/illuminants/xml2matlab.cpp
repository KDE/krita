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
because going to and from and back to a xml text representation of doubles loses precision.
 */

double **allocateMatrix(int m, int n)
{
    double **matrix;
    matrix = new double*[m];
    for (int i = 0; i < m; i++)
        matrix[i] = new double[n];
    return matrix;
}

bool CDataToDouble(QDomElement & e, double * d)
{

    if (!e.firstChild().isCDATASection()) {
        QTextStream(stderr) << "Element is not cdata " << e << endl;
        return false;
    }
    QDomCDATASection cdata = e.firstChild().toCDATASection();
    if (cdata.length() != 8) {
        QTextStream(stderr) << "Wrong number of bytes "  << cdata.length() << endl;
        return false;
    }

    QChar * data = cdata.data().data();

    memcpy(d, data, 8);

    return true;
}

bool readPrimaries(double *wavelengths, QDomElement primary, int numWavelengths)
{
    QDomElement wvl = primary.firstChildElement("wavelength");
    for (int i = 0; i < numWavelengths; i++) {
        if (wvl.isNull()) {
            QTextStream(stderr) << "Not enough wavelengths";
            return false;

        }
        QString v = wvl.attribute("value");
        if (v.isEmpty()) {
            QTextStream(stderr) << "No value for " << wvl << endl;
            return false;
        }
        bool ok = true;
        double d = v.toDouble(&ok);
        if (!ok) {
            QTextStream(stderr) << "Could not convert" << v << " to double." << endl;
            return false;
        }
        wavelengths[i] = d;
        wvl = wvl.nextSiblingElement("wavelength");
    }
    return true;
}

bool verifyCount(const QString & elementName, QDomElement & e, int num)
{
    // Check whether there are enough coeffs
    int count = 0;
    QDomElement c = e.firstChildElement(elementName);
    while (! c.isNull()) {
        ++count;
        c = c.nextSiblingElement(elementName);
    }
    return (count == num);

}

int main(int c, char **v)
{

    QCoreApplication app(c, v);

    if (sizeof(double) != 8) {
        QTextStream(stdout) << "double is the wrong size " << sizeof(double) << endl;
        return 1;
    }

    if (c < 3) {
        QTextStream(stdout) << "Usage; matlab2xml input output" << endl;
        return 1;
    }

    QFile inFile(app.arguments()[1]);
    if (! inFile.open(QIODevice::ReadOnly)) {
        QTextStream(stderr) << "Could not open file for reading; " << app.arguments()[1] << endl;
        return 1;
    }

    QFile outFile(app.arguments()[2]);
    if (! outFile.open(QIODevice::WriteOnly)) {
        QTextStream(stderr) << "Could not open file for writing; " << app.arguments()[2] << endl;
        return 1;
    }

    int m_wl = -1;
    double **m_T = 0;
    double *m_red = 0;
    double *m_green = 0;
    double *m_blue = 0;

    int nc = 0;
    double *coeffs = 0;

    QString m_illuminant = "";

    QDomDocument doc("illuminant");
    if (!doc.setContent(&inFile)) {
        QTextStream(stderr) << "Not an XML file; " << app.arguments()[1] << endl;
        inFile.close();
        return 1;
    }

    QDomElement root = doc.documentElement();
    if (root.nodeName() != "illuminant" || root.attribute("version") != "1") {
        QTextStream(stderr) << "Not an illuminant file or wrong version; " << app.arguments()[1] << endl;
        return 1;
    }

    m_wl = root.attribute("wavelengths").toInt();
    if (m_wl == 0) {
        QTextStream(stderr) << "No wavelengths; " << app.arguments()[1] << endl;
        return 1;
    }
    m_illuminant = root.attribute("name");
    if (m_illuminant.isEmpty()) {
        QTextStream(stderr) << "No name; " << app.arguments()[1] << endl;
        return 1;
    }
    m_T = allocateMatrix(3, m_wl);
    QDomElement transformations = root.firstChildElement("transformations");
    if (transformations.isNull()) {
        QTextStream(stderr) << "No transformations element; " << app.arguments()[1] << endl;
        return 1;
    }
    if (!verifyCount("transformation", transformations, 3 * m_wl)) {
        QTextStream(stderr) << "Wrong number of transformations; " << app.arguments()[1] << endl;
        return 1;
    }

    QDomElement transformation = transformations.firstChildElement("transformation");
    for (int i = 0; i < m_wl; i++) {
        for (int j = 0; j < 3; j++) {
            if (transformation.isNull()) {
                QTextStream(stderr) << "Not enough transformations: " << app.arguments()[1] << endl;
                return 1;

            }
            QString v = transformation.attribute("value");
            if (v.isEmpty()) {
                QTextStream(stderr) << "No value for " << transformation << app.arguments()[1] << endl;
                return 0;
            }
            bool ok = true;
            double d = v.toDouble(&ok);
            if (!ok) {
                QTextStream(stderr) << "Could not convert" << v << " to double: " << app.arguments()[1] << endl;
                return 0;
            }
            m_T[j][i] = d;
            transformation = transformation.nextSiblingElement("transformation");
        }
    }



    QDomElement primaries = root.firstChildElement("primaries");
    if (primaries.isNull()) {
        QTextStream(stderr) << "No primaries; " << app.arguments()[1] << endl;
        return 1;
    }

    m_red   = new double[m_wl];
    QDomElement red = primaries.firstChildElement("red");
    if (red.isNull()) {
        QTextStream(stderr) << "No red; " << app.arguments()[1] << endl;
        return 1;
    }

    if (!verifyCount("wavelength", red, m_wl)) {
        QTextStream(stderr) << "Wrong number of red wavelengths; " << m_wl << ": " << app.arguments()[1] << endl;
        return 1;
    }

    readPrimaries(m_red, red, m_wl);

    m_green = new double[m_wl];
    QDomElement green = primaries.firstChildElement("green");
    if (green.isNull()) {
        QTextStream(stderr) << "No green; " << app.arguments()[1] << endl;
        return 1;
    }
    if (!verifyCount("wavelength", green, m_wl)) {
        QTextStream(stderr) << "Wrong number of green wavelengths; " << app.arguments()[1] << endl;
        return 1;
    }
    readPrimaries(m_green, green, m_wl);

    m_blue  = new double[m_wl];
    QDomElement blue = primaries.firstChildElement("blue");
    if (blue.isNull()) {
        QTextStream(stderr) << "No blue: " << app.arguments()[1] << endl;
        return 1;
    }
    if (!verifyCount("wavelength", blue, m_wl)) {
        QTextStream(stderr) << "Wrong number of blue wavelengths; " << app.arguments()[1] << endl;
        return 1;
    }
    readPrimaries(m_blue, blue, m_wl);

    QDomElement X = root.firstChildElement("X");
    if (X.isNull()) {
        QTextStream(stderr) << "No X: " << app.arguments()[1] << endl;
        return 1;
    }

    nc = X.attribute("nc").toInt();
    if (nc == 0) {
        QTextStream(stderr) << "No number of coefficients " << app.arguments()[1] << endl;
        return 1;
    }

    if (!verifyCount("coefficient", X, m_wl * nc)) {
        QTextStream(stderr) << "Got wrong number coefficients (" << nc * m_wl << "): " << app.arguments()[1] << endl;
        return 1;
    }

    coeffs = new double[nc*m_wl];


    QDomElement coefficient = X.firstChildElement("coefficient");
    for (int i = 0; i < nc * m_wl; ++i) {
        if (coefficient.isNull()) {
            QTextStream(stderr) << "Got wrong number of coefficients";
            return 1;
        }
        QString v = coefficient.attribute("value");
        if (v.isEmpty()) {
            QTextStream(stderr) << "No value for " << coefficient << app.arguments()[1] << endl;
            return 0;
        }
        bool ok = true;
        double d = v.toDouble(&ok);
        if (!ok) {
            QTextStream(stderr) << "Could not convert" << v << " to double: " << app.arguments()[1] << endl;
            return 0;
        }

        coeffs[i] = d;

        coefficient = coefficient.nextSiblingElement("coefficient");
    }

    QDataStream data(&outFile);

    for (int i = 0; i < m_illuminant.size(); i++)
        data << (qint8)m_illuminant[i].toAscii();
    data << (qint8)0;

    data << (qint8)m_wl;
    for (int i = 0; i < m_wl; i++)
        for (int j = 0; j < 3; j++)
            data.writeRawData((char*)&m_T[j][i], 8);

    for (int i = 0; i < m_wl; i++)
        data.writeRawData((char*)&m_red[i], 8);
    for (int i = 0; i < m_wl; i++)
        data.writeRawData((char*)&m_green[i], 8);
    for (int i = 0; i < m_wl; i++)
        data.writeRawData((char*)&m_blue[i], 8);

    data << (qint8)nc;
    for (qint8 i = 0; i < nc*m_wl; i++)
        data.writeRawData((char*)&coeffs[i], 8);

    inFile.close();
    outFile.close();

}
