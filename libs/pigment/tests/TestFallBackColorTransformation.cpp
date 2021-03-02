/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "TestFallBackColorTransformation.h"

#include "KoColorTransformation.h"
#include <KoFallBackColorTransformation.h>
#include <KoColorSpaceRegistry.h>

#include <simpletest.h>

struct KoDummyColorTransformation : public KoColorTransformation
{
    KoDummyColorTransformation()
    {
      m_parameters << 1 << 2;
    }
    QList<QVariant> m_parameters;
    void transform(const quint8 */*src*/, quint8 */*dst*/, qint32 /*nPixels*/) const override
    {
    }
    QList<QString> parameters() const override
    {
      QList<QString> s;
      s << "test";
      return s;
    }
    int parameterId(const QString& name) const override
    {
      if(name == "test")
      {
        return 1;
      } else {
        return 0;
      }
    }
    void setParameter(int id, const QVariant& parameter) override
    {
      m_parameters[id] = parameter;
    }
};

void TestFallBackColorTransformation::parametersForward()
{
  KoDummyColorTransformation* dummy = new KoDummyColorTransformation;
  KoFallBackColorTransformation* fallback = new KoFallBackColorTransformation(KoColorSpaceRegistry::instance()->rgb8(),
                                                                              KoColorSpaceRegistry::instance()->rgb16(),
                                                                              dummy);
  QCOMPARE(fallback->parameters().size(), 1);
  QCOMPARE(fallback->parameters()[0], QString("test"));
  QCOMPARE(fallback->parameterId("test"), 1);
  QCOMPARE(fallback->parameterId("other"), 0);
  fallback->setParameter(0, -1);
  fallback->setParameter(1, "value");
  QCOMPARE(dummy->m_parameters[0], QVariant(-1));
  QCOMPARE(dummy->m_parameters[1], QVariant("value"));
  delete fallback;
}

QTEST_GUILESS_MAIN(TestFallBackColorTransformation)
