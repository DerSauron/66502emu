/*
 * Copyright (c) 2021 Daniel Volk <mail@volkarts.com>
 *
 * This work is licensed under the terms of the MIT license.
 * For a copy, see LICENSE or <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <QSet>
#include <QSharedDataPointer>

class Program
{
public:
    class SourceLine
    {
    public:
        int line;
        int address;
        QChar type;
        QString text;
    };

public:
    Program();
    ~Program();

    Program(const Program& other);
    Program& operator=(const Program& other);

    bool isNull() const;
    bool hasSources() const;

    const QByteArray& binaryData() const;
    void setBinaryData(const QByteArray& binaryData);

    const QList<SourceLine>& sourceLines() const;
    void setSourceLines(const QList<SourceLine>& sourceLines);

private:
    class Data : public QSharedData
    {
    public:
        Data() {}
        Data(const Data& other) : QSharedData(other) {}
        virtual ~Data() {}

        QByteArray binrayData;
        QList<SourceLine> sourceLines;
    };
    QSharedDataPointer<Data> d;
};
