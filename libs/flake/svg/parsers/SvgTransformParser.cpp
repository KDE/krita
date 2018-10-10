/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "SvgTransformParser.h"

#include <QtGlobal>

//#include "kis_debug.h"

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>


namespace Private
{

struct matrix
{
    qreal a = 0;
    qreal b = 0;
    qreal c = 0;
    qreal d = 0;
    qreal e = 0;
    qreal f = 0;
};

struct translate
{
    qreal tx = 0.0;
    qreal ty = 0.0;
};

struct scale
{
    qreal sx = 0;
    qreal sy = 0;
    bool syPresent = false;
};

struct rotate
{
    qreal angle = 0;
    qreal cx = 0;
    qreal cy = 0;
};

struct skewX
{
    qreal angle = 0;
};

struct skewY
{
    qreal angle = 0;
};

struct transform_unit
{
    transform_unit() {}

    transform_unit(const matrix &m)
    : transform(QTransform(m.a, m.b, m.c, m.d, m.e, m.f))
    {
    }

    transform_unit(const translate &t)
    : transform(QTransform::fromTranslate(t.tx, t.ty))
    {
    }

    transform_unit(const scale &sc)
    : transform(QTransform::fromScale(sc.sx, sc.syPresent ? sc.sy : sc.sx))
    {
    }

    transform_unit(const rotate &r) {
        transform.rotate(r.angle);
        if (r.cx != 0.0 || r.cy != 0.0) {
            transform =
                QTransform::fromTranslate(-r.cx, -r.cy) *
                transform *
                QTransform::fromTranslate(r.cx, r.cy);
        }
    }

    transform_unit(const skewX &sx) {
        const qreal deg2rad = qreal(0.017453292519943295769);
        const qreal value = tan(deg2rad * sx.angle);
        transform.shear(value, 0);
    }

    transform_unit(const skewY &sy) {
        const qreal deg2rad = qreal(0.017453292519943295769);
        const qreal value = tan(deg2rad * sy.angle);
        transform.shear(0, value);
    }

    QTransform transform;
};
}

// We need to tell fusion about our transform_unit struct
// to make it a first-class fusion citizen. This has to
// be in global scope.

BOOST_FUSION_ADAPT_STRUCT(
        Private::matrix,
        (qreal, a)
        (qreal, b)
        (qreal, c)
        (qreal, d)
        (qreal, e)
        (qreal, f)
        )

BOOST_FUSION_ADAPT_STRUCT(
        Private::translate,
        (qreal, tx)
        (qreal, ty)
        )

BOOST_FUSION_ADAPT_STRUCT(
        Private::scale,
        (qreal, sx)
        (qreal, sy)
        (bool, syPresent)
        )

BOOST_FUSION_ADAPT_STRUCT(
        Private::rotate,
        (qreal, angle)
        (qreal, cx)
        (qreal, cy)
        )

BOOST_FUSION_ADAPT_STRUCT(
        Private::skewX,
        (qreal, angle)
        )

BOOST_FUSION_ADAPT_STRUCT(
        Private::skewY,
        (qreal, angle)
        )

#define BOOST_SPIRIT_DEBUG 1

namespace Private
{
    // Define our grammar

    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    template <typename Iterator>
    struct transform_unit_parser : qi::grammar<Iterator, std::vector<transform_unit>(), ascii::space_type>
    {
        transform_unit_parser() : transform_unit_parser::base_type(start)
        {
            namespace phoenix = boost::phoenix;
            using qi::lit;
            using qi::double_;
            using ascii::char_;
            using qi::cntrl;
            using phoenix::at_c;
            using phoenix::push_back;
            using namespace qi::labels;


            comma %= -char_(',');

            matrix_rule %=
                lit("matrix")
                >> '('
                >>  double_ >> comma
                >>  double_ >> comma
                >>  double_ >> comma
                >>  double_ >> comma
                >>  double_ >> comma
                >>  double_ >> comma
                >>  ')';

            translate_rule %=
                lit("translate")
                >> '(' >>  double_ >> comma >>  -double_ >>  ')';

            scale_rule %=
                lit("scale")
                >> '('
                >>  double_ >> comma
                >>  -double_ [at_c<2>(_val) = true]
                >>  ')';


            // due to braces "-(...)" we cannot use automated
            // semantic actions without relayouting the structure
            rotate_rule =
                lit("rotate")
                >> '('
                >>  double_ [at_c<0>(_val) = _1]
                >> comma
                >>  -(double_ [at_c<1>(_val) = _1]
                      >> comma
                      >> double_ [at_c<2>(_val) = _1])
                >>  ')';

            skewX_rule %= lit("skewX") >> '(' >>  double_ >>  ')';
            skewY_rule %= lit("skewY") >> '(' >>  double_ >>  ')';

            start %=
                (matrix_rule | translate_rule | scale_rule |
                 rotate_rule | skewX_rule | skewY_rule) %
                (cntrl | comma);
        }

        qi::rule<Iterator, std::vector<transform_unit>(), ascii::space_type> start;
        qi::rule<Iterator, translate(), ascii::space_type> translate_rule;
        qi::rule<Iterator, matrix(), ascii::space_type> matrix_rule;
        qi::rule<Iterator, scale(), ascii::space_type> scale_rule;
        qi::rule<Iterator, rotate(), ascii::space_type> rotate_rule;
        qi::rule<Iterator, skewX(), ascii::space_type> skewX_rule;
        qi::rule<Iterator, skewY(), ascii::space_type> skewY_rule;
        qi::rule<Iterator> comma;
    };
}


SvgTransformParser::SvgTransformParser(const QString &_str)
    : m_isValid(false)
{
    using boost::spirit::ascii::space;
    typedef std::string::const_iterator iterator_type;
    typedef Private::transform_unit_parser<iterator_type> transform_unit_parser;

    transform_unit_parser g; // Our grammar
    const std::string str = _str.toStdString();

    std::vector<Private::transform_unit> transforms;
    iterator_type iter = str.begin();
    iterator_type end = str.end();
    bool r = phrase_parse(iter, end, g, space, transforms);

    if (r && iter == end) {
        m_isValid = true;

        for (const Private::transform_unit &t : transforms) {
             m_transform = t.transform * m_transform;
         }
    }
}
bool SvgTransformParser::isValid() const
{
    return m_isValid;
}

QTransform SvgTransformParser::transform() const
{
    return m_transform;
}


