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

#include "LooseSignal.h"

namespace internal {

void SlotProxy::onAction()
{
    auto signalProxy = qobject_cast<SignalProxy*>(sender());
    if (!signalProxy)
        return;
    signalProxy->resetCallCounter();
    callback_();
}

void SignalProxy::resetCallCounter()
{
    callCounter_ = 0;
}

void SignalProxy::trigger()
{
    auto lastCount = callCounter_.fetchAndAddOrdered(1);
    if (lastCount == 0)
        emit action();
}

} // namespace internal
