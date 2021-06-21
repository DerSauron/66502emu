/*
 * Copyright (C) 2021 Daniel Volk <mail@volkarts.com>
 *
 * This file is part of 6502emu - 6502 cycle accurate emulator gui.
 *
 * 6502emu is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License
 * along with 6502emu. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QAtomicInteger>
#include <QObject>

class LooseSignal;

namespace internal {

class SlotProxy : public QObject
{
    Q_OBJECT

public:
    ~SlotProxy() override = default;

private slots:
    void onAction();

private:
    template<typename SlotFunc>
    explicit SlotProxy(typename QtPrivate::FunctionPointer<SlotFunc>::Object* receiver, SlotFunc slot) :
        QObject(receiver),
//        callback_([slot, receiver]() { std::invoke(slot, receiver); })
        callback_(std::bind(slot, receiver))
    {
    }

private:
    std::function<void()> callback_;

    friend LooseSignal;
};

class SignalProxy : public QObject
{
    Q_OBJECT

public:
    ~SignalProxy() override = default;

signals:
    void action();

private:
    explicit SignalProxy(QObject* parent)
    {
        moveToThread(parent->thread());
        setParent(parent);
    }

    void resetCallCounter();

private slots:
    void trigger();

private:
    QAtomicInteger<uint64_t> callCounter_;

    friend LooseSignal;
    friend SlotProxy;
};

} // namespace internal

class LooseSignal
{
public:
    template <typename Func1, typename Func2>
    static inline void connect(
            typename QtPrivate::FunctionPointer<Func1>::Object* sender, Func1 signal,
            typename QtPrivate::FunctionPointer<Func2>::Object* receiver, Func2 slot)
    {
        typedef QtPrivate::FunctionPointer<Func1> SignalType;
        typedef QtPrivate::FunctionPointer<Func2> SlotType;

        Q_STATIC_ASSERT_X(int(SignalType::ArgumentCount) == 0 && int(SlotType::ArgumentCount) == 0,
                          "Slot and/or signal cannot have parameters.");

        auto* signalProxy = new internal::SignalProxy(sender);
        auto* slotProxy = new internal::SlotProxy(receiver, std::forward<Func2>(slot));

        QObject::connect(sender, std::forward<Func1>(signal),
                         signalProxy, &internal::SignalProxy::trigger, Qt::DirectConnection);
        QObject::connect(signalProxy, &internal::SignalProxy::action,
                         slotProxy, &internal::SlotProxy::onAction, Qt::QueuedConnection);
    }

private:
    LooseSignal() = delete;
};
