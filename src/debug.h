#pragma once

#define BP_MAP_SIZE (0x10000 / 8) // $FFFF address space divided by 8 bits per index

class Nes;

extern "C"
{
	extern __declspec(dllexport) int g_dbgEnableLoadEvent;
}

typedef enum
{
    DEBUG_BP_READ = 1,
    DEBUG_BP_WRITE = 2,
    DEBUG_BP_EXECUTE = 3
} BreakpointKind;

// Debug events
enum
{
	DEBUG_EVENT_NONE = 0,
    DEBUG_EVENT_BP_READ = 1,
    DEBUG_EVENT_BP_WRITE = 2,
    DEBUG_EVENT_BP_EXECUTE = 3,
    DEBUG_EVENT_LOAD_ROM = 4,
    DEBUG_EVENT_SINGLE_STEP = 5,
    DEBUG_EVENT_ILLEGAL_INSTRUCTION = 5
};

struct DebugEventInfo
{
	u64 _instance;
	int _eventId;
	int _eventParam;
};

class DebugService : public IBaseInterface, public NesObject
{
public:
#ifdef DAC_BUILD
	DebugService();
#else
    DebugService(Nes* nes);
#endif

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

    void EnableSingleStep();
    void SetBreakpoint(BreakpointKind kind, u16 address);
    void ClearBreakpoint(BreakpointKind kind, u16 address);

    void OnBeforeExecuteInstruction(u16 pc);

	// Debug API
public:
	bool FilterException(u64 param0, u64 param1, u64 param2, u64 param3, DebugEventInfo* eventInfo);
	u32 GetMapperNumber();

private:
	DPtr<Nes> _nes;
    u8 _bpMapRead[BP_MAP_SIZE];
    u8 _bpMapWrite[BP_MAP_SIZE];
    u8 _bpMapExecute[BP_MAP_SIZE];
    bool _singleStep;

    static u16 BreakpointIndex(u16 address)
    {
        return address / 8;
    }

    static u8 BreakpointBit(u16 address)
    {
        return 1 << (address & 7);
    }

	void DebuggerNotify(int dbgEvent, int param);
};

#undef BP_MAP_SIZE