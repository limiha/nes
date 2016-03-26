
#include "stdafx.h"
#include "debug.h"

int g_dbgEvent = 0;
int g_dbgParam = 0;
int g_dbgEnableLoadEvent = 0;

void __declspec(noinline) DebuggerNotify(int dbgEvent, int param)
{
    g_dbgEvent = dbgEvent;
    g_dbgParam = param;
    __debugbreak();
}

DebugService::DebugService()
    : _singleStep(false)
{
    memset(&_bpMapRead, 0, sizeof(_bpMapRead));
    memset(&_bpMapWrite, 0, sizeof(_bpMapWrite));
    memset(&_bpMapExecute, 0, sizeof(_bpMapExecute));

    if (g_dbgEnableLoadEvent)
    {
        DebuggerNotify(DEBUG_EVENT_LOAD_ROM, (int)this);
    }
}

void DebugService::SetBreakpoint(BreakpointKind kind, u16 address)
{
    u16 index = BreakpointIndex(address);
    u8 bit = BreakpointBit(address);
    switch (kind)
    {
    case DEBUG_BP_READ:
        _bpMapRead[index] |= bit;
        break;
    case DEBUG_BP_WRITE:
        _bpMapWrite[index] |= bit;
        break;
    case DEBUG_BP_EXECUTE:
        _bpMapExecute[index] |= bit;
        break;
    }
}

void DebugService::ClearBreakpoint(BreakpointKind kind, u16 address)
{
    u16 index = BreakpointIndex(address);
    u8 mask = ~BreakpointBit(address);
    switch (kind)
    {
    case DEBUG_BP_READ:
        _bpMapRead[index] &= mask;
        break;
    case DEBUG_BP_WRITE:
        _bpMapWrite[index] &= mask;
        break;
    case DEBUG_BP_EXECUTE:
        _bpMapExecute[index] &= mask;
        break;
    }
}

void DebugService::OnBeforeExecuteInstruction(u16 pc)
{
    u16 index = BreakpointIndex(pc);
    u8 bit = BreakpointBit(pc);

    if (_singleStep)
    {
        DebuggerNotify(DEBUG_EVENT_SINGLE_STEP, 0);
        _singleStep = false;
    }

    if (_bpMapExecute[index] & bit)
    {
        DebuggerNotify(DEBUG_EVENT_BP_EXECUTE, pc);
    }
}