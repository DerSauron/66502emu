/*
 * Copyright (c) 2021 Daniel Volk <mail@volkarts.com>
 *
 * This work is licensed under the terms of the MIT license.
 * For a copy, see LICENSE or <https://opensource.org/licenses/MIT>.
 */

#include "Highlighter.h"

namespace ce {

Highlighter::Highlighter(QTextDocument* parent) :
    QSyntaxHighlighter(parent)
{
}

Highlighter::~Highlighter()
{
}

void Highlighter::highlightBlock(const QString& text)
{

}

} // namespace ce
