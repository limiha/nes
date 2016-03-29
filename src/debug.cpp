
#include "stdafx.h"
#include "debug.h"
#include "nes.h"

#define NESDBG_EX_CODE  0x1983
#define NESDBG_MAGIC	0x19832016

int g_dbgEnableLoadEvent = 0;

#ifdef DAC_BUILD

DebugService::DebugService()
	: _singleStep(false)
{
	memset(&_bpMapRead, 0, sizeof(_bpMapRead));
	memset(&_bpMapWrite, 0, sizeof(_bpMapWrite));
	memset(&_bpMapExecute, 0, sizeof(_bpMapExecute));
}

#else

DebugService::DebugService(Nes* nes)
    : _nes(nes)
	, _singleStep(false)
{
    memset(&_bpMapRead, 0, sizeof(_bpMapRead));
    memset(&_bpMapWrite, 0, sizeof(_bpMapWrite));
    memset(&_bpMapExecute, 0, sizeof(_bpMapExecute));

    if (g_dbgEnableLoadEvent)
    {
        DebuggerNotify(DEBUG_EVENT_LOAD_ROM, (int)this);
    }
}

#endif

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

bool DebugService::FilterException(u64 param0, u64 param1, u64 param2, u64 param3, DebugEventInfo* eventInfo)
{
	if (param0 != NESDBG_MAGIC)
		return false; // Magic number doesn't match

	eventInfo->_instance = param1;
	eventInfo->_eventId = (int)param2;
	eventInfo->_eventParam = (int)param3;

	return true;
}

u32 DebugService::GetMapperNumber()
{
	return _nes->GetMapperNumber();
}

void DebugService::DebuggerNotify(int dbgEvent, int param)
{
	__try
	{
		ULONG_PTR args[4];
		args[0] = NESDBG_MAGIC;
		args[1] = (ULONG_PTR)this;
		args[2] = (ULONG_PTR)dbgEvent;
		args[3] = (ULONG_PTR)param;
		::RaiseException(
			NESDBG_EX_CODE,
			0,
			4 /* num arguments */,
			args);
	}
	__except (TRUE)
	{
	}
}