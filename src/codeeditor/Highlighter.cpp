/*
 * Copyright (c) 2021 Daniel Volk <mail@volkarts.com>
 *
 * This work is licensed under the terms of the MIT license.
 * For a copy, see LICENSE or <https://opensource.org/licenses/MIT>.
 */

#include "Highlighter.h"

namespace ce {

Highlighter::Highlighter(QTextDocument* parent) :
    QSyntaxHighlighter{parent}
{
}

Highlighter::~Highlighter()
{
}

void Highlighter::setRules(const QList<QString>& keywords, const QList<QString>& spechialWords1,
                           const QList<QString>& spechialWords2, const QList<QString>& numberModifiers,
                           const QList<QString>& lineCommentStarts)
{
    keywords_ = keywords;
    specialWords1_ = spechialWords1;
    specialWords2_ = spechialWords2;
    numberModifiers_ = numberModifiers;
    lineCommentStarts_ = lineCommentStarts;
    rebuildRules();
    rehighlight();
}

void Highlighter::highlightBlock(const QString& text)
{
    for (const auto& rule : qAsConst(rules_))
    {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext())
        {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

void Highlighter::rebuildRules()
{
    rules_.clear();

    Rule rule;
    QTextCharFormat format;

//    format.setForeground(Qt::darkCyan);
//    format.setFontWeight(QFont::Normal);
//    rule.pattern = QRegularExpression(QStringLiteral("[~!@#$%^&*\\/+\\-]+"));
//    rule.format = format;
//    rules_.append(rule);

//    format.setForeground(Qt::darkBlue);
//    format.setFontWeight(QFont::Normal);
//    rule.pattern = QRegularExpression(QStringLiteral("[(){}\\[\\]]+"));
//    rule.format = format;
//    rules_.append(rule);

    format.setForeground(Qt::darkGreen);
    format.setFontWeight(QFont::Normal);
    rule.pattern = QRegularExpression(QLatin1String("\"((\\\\{2})*|(.*?[^\\\\](\\\\{2})*))\""),
                                      QRegularExpression::CaseInsensitiveOption);
    rule.format = format;
    rules_.append(rule);

    format.setForeground(Qt::darkYellow);
    format.setFontWeight(QFont::Normal);
    const QString numberPattern =
            QLatin1String("(?<![0-9a-z_])(") + alternation(numberModifiers_) + QLatin1String("|)") +
            QStringLiteral("[0-9a-f]+(?![g-z_])");
    rule.pattern = QRegularExpression(numberPattern, QRegularExpression::CaseInsensitiveOption);
    rule.format = format;
    rules_.append(rule);

    format.setForeground(Qt::darkGreen);
    format.setFontWeight(QFont::Normal);
    const QString lineCommentsPattern =
            QLatin1String("(") + alternation(lineCommentStarts_) + QLatin1String(")") +
            QLatin1String(".*");
    rule.pattern = QRegularExpression(lineCommentsPattern, QRegularExpression::CaseInsensitiveOption);
    rule.format = format;
    rules_.append(rule);

    format.setForeground(Qt::darkMagenta);
    format.setFontWeight(QFont::Normal);
    for (const auto& word : qAsConst(specialWords2_))
    {
        rule.pattern = QRegularExpression(QLatin1String("\\b") + word + QLatin1String("\\b"),
                                          QRegularExpression::CaseInsensitiveOption);
        rule.format = format;
        rules_.append(rule);
    }

    format.setForeground(Qt::darkCyan);
    format.setFontWeight(QFont::Normal);
    for (const auto& word : qAsConst(specialWords1_))
    {
        rule.pattern = QRegularExpression(QLatin1String("\\b") + word + QLatin1String("\\b"),
                                          QRegularExpression::CaseInsensitiveOption);
        rule.format = format;
        rules_.append(rule);
    }


    format.setForeground(Qt::black);
    format.setFontWeight(QFont::Bold);
    for (const auto& word : qAsConst(keywords_))
    {
        rule.pattern = QRegularExpression(QLatin1String("\\b") + word + QLatin1String("\\b"),
                                          QRegularExpression::CaseInsensitiveOption);
        rule.format = format;
        rules_.append(rule);
    }
}

QString Highlighter::alternation(const QList<QString>& list)
{
    QString regexp;
    for (const auto& e : list)
    {
        if (!regexp.isEmpty())
            regexp += QLatin1String("|");
        regexp += QRegularExpression::escape(e);
    }
    return regexp;
}

} // namespace ce
