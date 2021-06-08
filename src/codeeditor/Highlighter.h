/*
 * Copyright (c) 2021 Daniel Volk <mail@volkarts.com>
 *
 * This work is licensed under the terms of the MIT license.
 * For a copy, see LICENSE or <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <QSyntaxHighlighter>
#include <QRegularExpression>

namespace ce {

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    Highlighter(QTextDocument* parent = nullptr);
    virtual ~Highlighter() override;

    void setRules(const QList<QString>& keywords, const QList<QString>& spechialWords1,
                  const QList<QString>& spechialWords2, const QList<QString>& numberModifiers,
                  const QList<QString>& lineCommentStarts);

protected:
    void highlightBlock(const QString& text) override;

private:
    struct Rule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

private:
    void rebuildRules();
    static QString alternation(const QList<QString>& list);

private:
    QList<QString> keywords_;
    QList<QString> specialWords1_;
    QList<QString> specialWords2_;
    QList<Rule> rules_;
    QList<QString> numberModifiers_;
    QList<QString> lineCommentStarts_;
};

} // namespace ce
