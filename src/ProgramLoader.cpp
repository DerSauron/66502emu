/*
 * Copyright (c) 2021 Daniel Volk <mail@volkarts.com>
 *
 * This work is licensed under the terms of the MIT license.
 * For a copy, see LICENSE or <https://opensource.org/licenses/MIT>.
 */

#include "ProgramLoader.h"

#include "Program.h"
#include <QDataStream>
#include <QBuffer>
#include <QDebug>
#include <QFile>

namespace {

class ListingParser
{
private:
    enum class State
    {
        None,
        Sections,
        Sources,
        SymbolsByName,
        SymbolsByValue,
    };

public:
    ListingParser()
    {
    }

    void startRun() { run_++; }

    bool parseLine(const QByteArray& line)
    {
        if (determineState(line))
            return true;

        switch (state_)
        {
            case State::Sections:
                return parseSectionLine(line);

            case State::Sources:
                return parseSourceLine(line);

            default:
                break;
        }

        return true;
    }

    bool needMoreRuns() const { return needMoreRuns_; }
    const QByteArray& binaryData() const { return binaryData_; }
    const QList<Program::SourceLine>& sourceLines() const { return sourceLines_; }

private:
    bool determineState(const QByteArray& line)
    {
        if (line.startsWith("Sections:"))
        {
            state_ = State::Sections;
            return true;
        }
        else if (line.startsWith("Source:"))
        {
            state_ = State::Sources;
            return true;
        }
        else if (line.startsWith("Symbols by name:"))
        {
            state_ = State::SymbolsByName;
            return true;
        }
        else if (line.startsWith("Symbols by value:"))
        {
            state_ = State::SymbolsByValue;
            return true;
        }

        return false;
    }

    bool parseSectionLine(const QByteArray& line)
    {
        if (sectionExpression_.indexIn(QString::fromUtf8(line)) == -1)
            return false;

        uint16_t startAddress = static_cast<uint16_t>(sectionExpression_.cap(2).toInt(nullptr, 16));
        if (startAddress < programStartAddress_)
        {
            programStartAddress_ = startAddress;
            hasProgramStartAddress_ = true;
        }

        return true;
    }

    bool parseSourceLine(const QByteArray& line)
    {
        if (!hasProgramStartAddress_)
        {
            if (run_ < 2)
            {
                needMoreRuns_ = true;
                return true;
            }
            else
            {
                qWarning() << "no sections found";
                return false;
            }
        }

        const QList<QByteArray> tokens = line.split('\t');

        Program::SourceLine sl;

        bool hasSourceLine = tokens.size() >= 2 && lineExpression_.indexIn(QString::fromUtf8(tokens[1])) != -1;
        if (hasSourceLine)
        {
            sl.line = lineExpression_.cap(1).toInt();
            sl.type = lineExpression_.cap(2).at(0);
            sl.text = lineExpression_.cap(3);
        }

        if (addrExpression_.indexIn(QString::fromUtf8(tokens[0])) != -1)
        {
            sl.address = addrExpression_.cap(2).toInt(nullptr, 16);
            uint16_t relAddr = static_cast<uint16_t>(sl.address - programStartAddress_);
            QByteArray data = QByteArray::fromHex(addrExpression_.cap(3).toUtf8());

            if (binaryData_.length() < relAddr + data.length())
                binaryData_.resize(relAddr + data.length());

            for (int i = 0; i < data.length(); i++)
            {
                binaryData_[relAddr + i] = data[i];
            }
        }
        else
        {
            sl.address = -1;
        }

        if (hasSourceLine)
            sourceLines_ << sl;

        return true;
    }

private:
    const QRegExp sectionExpression_{QStringLiteral("([0-9a-z]+):[^\\(]+\\(([0-9a-z]+)"), Qt::CaseInsensitive};
    const QRegExp lineExpression_{QStringLiteral("([0-9]+)(.)\\s(.*)"), Qt::CaseInsensitive};
    const QRegExp addrExpression_{QStringLiteral("([0-9a-z]+):([0-9a-z]+)\\s+([0-9a-z]+)"), Qt::CaseInsensitive};

    State state_{State::None};

    int run_{0};
    bool needMoreRuns_{false};
    QByteArray binaryData_;
    uint16_t programStartAddress_{std::numeric_limits<uint16_t>::max()};
    bool hasProgramStartAddress_{false};
    QList<Program::SourceLine> sourceLines_;
};

} //namespace

ProgramLoader::ProgramLoader(QObject* parent) :
    QObject(parent)
{
}

ProgramLoader::~ProgramLoader()
{
}

Program ProgramLoader::loadProgram(const QString& fileName)
{
    if (fileName.endsWith(QLatin1String(".lst")))
        return loadListing(fileName);
    else
        return loadBinary(fileName);
}

Program ProgramLoader::loadBinary(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
    {
        qWarning() << "Failed to open binary file" << fileName;
        return {};
    }

    Program blob;
    blob.setBinaryData(file.readAll());
    return blob;
}

Program ProgramLoader::loadListing(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        qWarning() << "Failed to open listing file" << fileName;
        return {};
    }

    int lineNo = 1;
    ListingParser parser;
    while (true)
    {
        parser.startRun();

        while (true)
        {
            QByteArray line = file.readLine();
            if (line.isEmpty())
                break;

            if (line.trimmed().isEmpty())
                continue;

            if (!parser.parseLine(line))
            {
                qWarning() << "Error while parsing listing file" << fileName << "at line" << lineNo;
                return {};
            }

            lineNo++;
        }

        if (!parser.needMoreRuns())
            break;

        bool seekOk = file.seek(0);
        if (!seekOk)
        {
            qWarning() << "Error while reset file position for second parser run";
            return {};
        }
    }

    Program program;
    program.setBinaryData(parser.binaryData());
    program.setSourceLines(parser.sourceLines());

    return program;
}
