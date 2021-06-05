/*
 * Copyright (c) 2021 Daniel Volk <mail@volkarts.com>
 *
 * This work is licensed under the terms of the MIT license.
 * For a copy, see LICENSE or <https://opensource.org/licenses/MIT>.
 */

#include "Program.h"

#include <QDebug>

Program::Program() :
    d(new Data())
{
}

Program::~Program()
{
}

Program::Program(const Program& other) :
    d(other.d)
{
}

Program& Program::operator=(const Program& other)
{
    d = other.d;
    return *this;
}

bool Program::isNull() const
{
    return d->binrayData.isEmpty();
}

bool Program::hasSources() const
{
    return !d->sourceLines.isEmpty();
}

const QByteArray& Program::binaryData() const
{
    return d->binrayData;
}

void Program::setBinaryData(const QByteArray& binaryData)
{
    d->binrayData = binaryData;
}

const QList<Program::SourceLine>& Program::sourceLines() const
{
    return d->sourceLines;
}

void Program::setSourceLines(const QList<SourceLine>& sourceLines)
{
    d->sourceLines = sourceLines;
}

bool Program::breakpointMatches(int address) const
{
    const auto pos = d->breakpoints.find(address);
    return pos != d->breakpoints.end();
}

void Program::addBreakpoint(int address)
{
    d->breakpoints.insert(address);
}

void Program::removeBreakpoint(int address)
{
    d->breakpoints.remove(address);
}

bool Program::toggleBreakpoint(int address)
{
    if (d->breakpoints.find(address) == d->breakpoints.end())
    {
        addBreakpoint(address);
        return true;
    }
    else
    {
        removeBreakpoint(address);
        return false;
    }
}
