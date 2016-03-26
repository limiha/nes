#pragma once

#define BP_MAP_SIZE (0x10000 / 8) // $FFFF address space divided by 8 bits per index

extern int g_dbgEvent;
extern int g_dbgParam;
extern int g_dbgEnableLoadEvent;

typedef enum
{
    DEBUG_BP_READ = 1,
    DEBUG_BP_WRITE = 2,
    DEBUG_BP_EXECUTE = 3
} BreakpointKind;

// Debug events
enum
{
    DEBUG_EVENT_BP_READ = 1,
    DEBUG_EVENT_BP_WRITE = 2,
    DEBUG_EVENT_BP_EXECUTE = 3,
    DEBUG_EVENT_LOAD_ROM = 4,
    DEBUG_EVENT_SINGLE_STEP = 5,
    DEBUG_EVENT_ILLEGAL_INSTRUCTION = 5
};

class DebugService : public IBaseInterface, public NesObject
{
public:
    DebugService();

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

    void EnableSingleStep();
    void SetBreakpoint(BreakpointKind kind, u16 address);
    void ClearBreakpoint(BreakpointKind kind, u16 address);

    void OnBeforeExecuteInstruction(u16 pc);

private:
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
};

#undef BP_MAP_SIZE