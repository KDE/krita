/* This file is part of the KDE project
 * Copyright (C) 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "SvgCssHelper.h"
#include <KoXmlReader.h>
#include <FlakeDebug.h>
#include <QPair>

/// Token types used for tokenizing complex selectors
enum CssTokenType {
    SelectorToken,  ///< a selector token
    CombinatorToken ///< a combinator token
};

/// A token used for tokenizing complex selectors
typedef QPair<CssTokenType, QString> CssToken;

/// Selector base class, merely an interface
class CssSelectorBase
{
public:
    virtual ~CssSelectorBase() {}
    /// Matches the given element
    virtual bool match(const KoXmlElement &) = 0;
    /// Returns string representation of selector
    virtual QString toString() const { return QString(); }
    /**
     * Returns priority of selector
     * see http://www.w3.org/TR/1998/REC-CSS2-19980512/cascade.html#specificity
     */
    virtual int priority() { return 0; }
};

/// Universal selector, matching anything
class UniversalSelector : public CssSelectorBase
{
public:
    bool match(const KoXmlElement &) override
    {
        // matches always
        return true;
    }
    QString toString() const override
    {
        return "*";
    }
};

/// Type selector, matching the type of an element
class TypeSelector : public CssSelectorBase
{
public:
    TypeSelector(const QString &type)
    : m_type(type)
    {
    }
    bool match(const KoXmlElement &e) override
    {
        return e.tagName() == m_type;
    }
    QString toString() const override
    {
        return m_type;
    }
    int priority() override
    {
        return 1;
    }

private:
    QString m_type;
};

/// Id selectdor, matching the id attribute
class IdSelector : public CssSelectorBase
{
public:
    IdSelector(const QString &id)
    : m_id(id)
    {
        if (id.startsWith('#'))
            m_id = id.mid(1);
    }
    bool match(const KoXmlElement &e) override
    {
        return e.attribute("id") == m_id;
    }
    QString toString() const override
    {
        return '#'+m_id;
    }
    int priority() override
    {
        return 100;
    }
private:
    QString m_id;
};

/// Attribute selector, matching existence or content of attributes
class AttributeSelector : public CssSelectorBase
{
public:
    AttributeSelector(const QString &attribute)
    : m_type(Unknown)
    {
        QString pattern = attribute;
        if (pattern.startsWith('['))
            pattern.remove(0,1);
        if (pattern.endsWith(']'))
            pattern.remove(pattern.length()-1,1);
        int equalPos = pattern.indexOf('=');
        if (equalPos == -1) {
            m_type = Exists;
            m_attribute = pattern;
        } else if (equalPos > 0){
            if (pattern[equalPos-1] == '~') {
                m_attribute = pattern.left(equalPos-1);
                m_type = InList;
            } else if(pattern[equalPos-1] == '|') {
                m_attribute = pattern.left(equalPos-1) + '-';
                m_type = StartsWith;
            } else {
                m_attribute = pattern.left(equalPos);
                m_type = Equals;
            }
            m_value = pattern.mid(equalPos+1);
            if (m_value.startsWith(QLatin1Char('"')))
                m_value.remove(0,1);
            if (m_value.endsWith(QLatin1Char('"')))
                m_value.chop(1);
        }
    }

    bool match(const KoXmlElement &e) override
    {
        switch(m_type) {
            case Exists:
                return e.hasAttribute(m_attribute);
                break;
            case Equals:
                return e.attribute(m_attribute) == m_value;
                break;
            case InList:
                {
                    QStringList tokens = e.attribute(m_attribute).split(' ', QString::SkipEmptyParts);
                    return tokens.contains(m_value);
                }
                break;
            case StartsWith:
                return e.attribute(m_attribute).startsWith(m_value);
                break;
            default:
                return false;
        }
    }
    QString toString() const override
    {
        QString str('[');
        str += m_attribute;
        if (m_type == Equals) {
            str += '=';
        } else if (m_type == InList) {
            str += "~=";
        } else if (m_type == StartsWith) {
            str += "|=";
        }
        str += m_value;
        str += ']';
        return str;
    }
    int priority() override
    {
        return 10;
    }

private:
    enum MatchType {
        Unknown,   ///< unknown    -> error state
        Exists,    ///< [att]      -> attribute exists
        Equals,    ///< [att=val]  -> attribute value matches exactly val
        InList,    ///< [att~=val] -> attribute is whitespace separated list where one is val
        StartsWith ///< [att|=val] -> attribute starts with val-
    };
    QString m_attribute;
    QString m_value;
    MatchType m_type;
};

/// Pseudo-class selector
class PseudoClassSelector : public CssSelectorBase
{
public:
    PseudoClassSelector(const QString &pseudoClass)
    : m_pseudoClass(pseudoClass)
    {
    }

    bool match(const KoXmlElement &e) override
    {
        if (m_pseudoClass == ":first-child") {
            KoXmlNode parent = e.parentNode();
            if (parent.isNull()) {
                return false;
            }
            KoXmlNode firstChild = parent.firstChild();
            while(!firstChild.isElement() || firstChild.isNull()) {
                firstChild = firstChild.nextSibling();
            }
            return firstChild == e;
        } else {
            return false;
        }
    }
    QString toString() const override
    {
        return m_pseudoClass;
    }
    int priority() override
    {
        return 10;
    }

private:
    QString m_pseudoClass;
};

/// A simple selector, i.e. a type/universal selector followed by attribute, id or pseudo-class selectors
class CssSimpleSelector : public CssSelectorBase
{
public:
    CssSimpleSelector(const QString &token)
    : m_token(token)
    {
        compile();
    }
    ~CssSimpleSelector() override
    {
        qDeleteAll(m_selectors);
    }

    bool match(const KoXmlElement &e) override
    {
        Q_FOREACH (CssSelectorBase *s, m_selectors) {
            if (!s->match(e))
                return false;
        }

        return true;
    }

    QString toString() const override
    {
        QString str;
        Q_FOREACH (CssSelectorBase *s, m_selectors) {
            str += s->toString();
        }
        return str;
    }
    int priority() override
    {
        int p = 0;
        Q_FOREACH (CssSelectorBase *s, m_selectors) {
            p += s->priority();
        }
        return p;
    }

private:
    void compile()
    {
        if (m_token == "*") {
            m_selectors.append(new UniversalSelector());
            return;
        }

        enum {
            Start,
            Finish,
            Bad,
            InType,
            InId,
            InAttribute,
            InClassAttribute,
            InPseudoClass
        } state;

        // add terminator to string
        QString expr = m_token + QChar();
        int i = 0;
        state = Start;

        QString token;
        QString sep("#[:.");
        // split into base selectors
        while((state != Finish) && (state != Bad) && (i < expr.length())) {
            QChar ch = expr[i];
            switch(state) {
                case Start:
                    token += ch;
                    if (ch == '#')
                        state = InId;
                    else if (ch == '[')
                        state = InAttribute;
                    else if (ch == ':')
                        state = InPseudoClass;
                    else if (ch == '.')
                        state = InClassAttribute;
                    else if (ch != '*')
                        state = InType;
                    break;
                case InAttribute:
                    if (ch.isNull()) {
                        // reset state and token string
                        state = Finish;
                        token.clear();
                        continue;
                    } else {
                        token += ch;
                        if (ch == ']') {
                            m_selectors.append(new AttributeSelector(token));
                            state = Start;
                            token.clear();
                        }
                    }
                    break;
                case InType:
                case InId:
                case InClassAttribute:
                case InPseudoClass:
                    // are we at the start of the next selector or even finished?
                    if (sep.contains(ch) || ch.isNull()) {
                        if (state == InType)
                            m_selectors.append(new TypeSelector(token));
                        else if (state == InId)
                            m_selectors.append(new IdSelector(token));
                        else if ( state == InClassAttribute)
                            m_selectors.append(new AttributeSelector("[class~="+token.mid(1)+']'));
                        else if (state == InPseudoClass) {
                            m_selectors.append(new PseudoClassSelector(token));
                        }
                        // reset state and token string
                        state = ch.isNull() ? Finish : Start;
                        token.clear();
                        continue;
                    } else {
                        // append character to current token
                        if (!ch.isNull())
                            token += ch;
                    }
                    break;
                case Bad:
                default:
                    break;
            }
            i++;
        }
    }

    QList<CssSelectorBase*> m_selectors;
    QString m_token;
};

/// Complex selector, i.e. a combination of simple selectors
class CssComplexSelector : public CssSelectorBase
{
public:
    CssComplexSelector(const QList<CssToken> &tokens)
    {
        compile(tokens);
    }
    ~CssComplexSelector() override
    {
        qDeleteAll(m_selectors);
    }
    QString toString() const override
    {
        QString str;
        int selectorCount = m_selectors.count();
        if (selectorCount) {
            for(int i = 0; i < selectorCount-1; ++i) {
                str += m_selectors[i]->toString() +
                       m_combinators[i];
            }
            str += m_selectors.last()->toString();
        }
        return str;
    }

    bool match(const KoXmlElement &e) override
    {
        int selectorCount = m_selectors.count();
        int combinatorCount = m_combinators.length();
        // check count of selectors and combinators
        if (selectorCount-combinatorCount != 1)
            return false;

        KoXmlElement currentElement = e;

        // match in reverse order
        for(int i = 0; i < selectorCount; ++i) {
            CssSelectorBase * curr = m_selectors[selectorCount-1-i];
            if (!curr->match(currentElement)) {
                return false;
            }
            // last selector and still there -> rule matched completely
            if(i == selectorCount-1)
                return true;

            CssSelectorBase * next = m_selectors[selectorCount-1-i-1];
            QChar combinator = m_combinators[combinatorCount-1-i];
            if (combinator == ' ') {
                bool matched = false;
                // descendant combinator
                KoXmlNode parent = currentElement.parentNode();
                while(!parent.isNull()) {
                    currentElement = parent.toElement();
                    if (next->match(currentElement)) {
                        matched = true;
                        break;
                    }
                    parent = currentElement.parentNode();
                }
                if(!matched)
                    return false;
            } else if (combinator == '>') {
                // child selector
                KoXmlNode parent = currentElement.parentNode();
                if (parent.isNull())
                    return false;
                KoXmlElement parentElement = parent.toElement();
                if (next->match(parentElement)) {
                    currentElement = parentElement;
                } else {
                    return false;
                }
            } else if (combinator == '+') {
                KoXmlNode neighbor = currentElement.previousSibling();
                while(!neighbor.isNull() && !neighbor.isElement())
                    neighbor = neighbor.previousSibling();
                if (neighbor.isNull() || !neighbor.isElement())
                    return false;
                KoXmlElement neighborElement = neighbor.toElement();
                if (next->match(neighborElement)) {
                    currentElement = neighborElement;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        }
        return true;
    }
    int priority() override
    {
        int p = 0;
        Q_FOREACH (CssSelectorBase *s, m_selectors) {
            p += s->priority();
        }
        return p;
    }

private:
    void compile(const QList<CssToken> &tokens)
    {
        Q_FOREACH (const CssToken &token, tokens) {
            if(token.first == SelectorToken) {
                m_selectors.append(new CssSimpleSelector(token.second));
            } else {
                m_combinators += token.second;
            }
        }
    }

    QString m_combinators;
    QList<CssSelectorBase*> m_selectors;
};

/// A group of selectors (comma separated in css style sheet)
typedef QList<CssSelectorBase*> SelectorGroup;
/// A css rule consisting of group of selectors corresponding to a style
typedef QPair<SelectorGroup, QString> CssRule;

class SvgCssHelper::Private
{
public:
    ~Private()
    {
        Q_FOREACH (const CssRule &rule, cssRules) {
            qDeleteAll(rule.first);
        }
    }

    SelectorGroup parsePattern(const QString &pattern)
    {
        SelectorGroup group;

        QStringList selectors = pattern.split(',', QString::SkipEmptyParts);
        for (int i = 0; i < selectors.count(); ++i ) {
            CssSelectorBase * selector = compileSelector(selectors[i].simplified());
            if (selector)
                group.append(selector);
        }
        return group;
    }

    QList<CssToken> tokenize(const QString &selector)
    {
        // add terminator to string
        QString expr = selector + QChar();
        enum {
            Finish,
            Bad,
            InCombinator,
            InSelector
        } state;

        QChar combinator;
        int selectorStart = 0;

        QList<CssToken> tokenList;

        QChar ch = expr[0];
        if (ch.isSpace() || ch == '>' || ch == '+') {
            debugFlake << "selector starting with combinator is not allowed:" << selector;
            return tokenList;
        } else {
            state = InSelector;
            selectorStart = 0;
        }
        int i = 1;

        // split into simple selectors and combinators
        while((state != Finish) && (state != Bad) && (i < expr.length())) {
            QChar ch = expr[i];
            switch(state) {
                case InCombinator:
                    // consume as long as there a combinator characters
                    if( ch == '>' || ch == '+') {
                        if( ! combinator.isSpace() ) {
                            // two non whitespace combinators in sequence are not allowed
                            state = Bad;
                        } else {
                            // switch combinator
                            combinator = ch;
                        }
                    } else if (!ch.isSpace()) {
                        tokenList.append(CssToken(CombinatorToken, combinator));
                        state = InSelector;
                        selectorStart = i;
                        combinator = QChar();
                    }
                    break;
                case InSelector:
                    // consume as long as there a non combinator characters
                    if (ch.isSpace() || ch == '>' || ch == '+') {
                        state = InCombinator;
                        combinator = ch;
                    } else if (ch.isNull()) {
                        state = Finish;
                    }
                    if (state != InSelector) {
                        QString simpleSelector = selector.mid(selectorStart, i-selectorStart);
                        tokenList.append(CssToken(SelectorToken, simpleSelector));
                    }
                    break;
                default:
                    break;
            }
            i++;
        }

        return tokenList;
    }

    CssSelectorBase * compileSelector(const QString &selector)
    {
        QList<CssToken> tokenList = tokenize(selector);
        if (tokenList.isEmpty())
            return 0;

        if (tokenList.count() == 1) {
            // simple selector
            return new CssSimpleSelector(tokenList.first().second);
        } else if (tokenList.count() > 2) {
            // complex selector
            return new CssComplexSelector(tokenList);
        }
        return 0;
    }

    QMap<QString, QString> cssStyles;
    QList<CssRule> cssRules;
};

SvgCssHelper::SvgCssHelper()
: d(new Private())
{
}

SvgCssHelper::~SvgCssHelper()
{
    delete d;
}

void SvgCssHelper::parseStylesheet(const KoXmlElement &e)
{
    QString data;

    if (e.hasChildNodes()) {
        KoXmlNode c = e.firstChild();
        if (c.isCDATASection()) {
            KoXmlCDATASection cdata = c.toCDATASection();
            data = cdata.data().simplified();
        } else if (c.isText()) {
            KoXmlText text = c.toText();
            data = text.data().simplified();
        }
    }
    if (data.isEmpty())
        return;

    // remove comments
    QRegExp commentExp("\\/\\*.*\\*\\/");
    commentExp.setMinimal(true); // do not match greedy
    data.remove(commentExp);

    QStringList defs = data.split('}', QString::SkipEmptyParts);
    for (int i = 0; i < defs.count(); ++i) {
        QStringList def = defs[i].split('{');
        if( def.count() != 2 )
            continue;
        QString pattern = def[0].simplified();
        if (pattern.isEmpty())
            break;
        QString style = def[1].simplified();
        if (style.isEmpty())
            break;
        QStringList selectors = pattern.split(',', QString::SkipEmptyParts);
        for (int i = 0; i < selectors.count(); ++i ) {
            QString selector = selectors[i].simplified();
            d->cssStyles[selector] = style;
        }
        SelectorGroup group = d->parsePattern(pattern);
        d->cssRules.append(CssRule(group, style));
    }
}

QStringList SvgCssHelper::matchStyles(const KoXmlElement &element) const
{
    QMap<int, QString> prioritizedRules;
    // match rules to element
    Q_FOREACH (const CssRule &rule, d->cssRules) {
        Q_FOREACH (CssSelectorBase *s, rule.first) {
            bool matched = s->match(element);
            if (matched)
                prioritizedRules[s->priority()] = rule.second;
        }
    }

    // css style attribute has the priority of 100
    QString styleAttribute = element.attribute("style").simplified();
    if (!styleAttribute.isEmpty())
        prioritizedRules[100] = styleAttribute;

    QStringList cssStyles;
    // add matching styles in correct order to style list
    QMapIterator<int, QString> it(prioritizedRules);
    while (it.hasNext()) {
        it.next();
        cssStyles.append(it.value());
    }

    return cssStyles;
}
