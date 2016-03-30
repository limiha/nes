#pragma once

#include "mem.h"

class AudioEngine;
struct IAudioProvider;
struct ApuPulseState;
struct ApuTriangleState;
struct ApuNoiseState;
struct ApuDmcState;
struct ApuEnvelop;

struct ApuStepResult
{
    bool Irq;

    void Reset()
    {
        Irq = false;
    }
};

class Apu : public IMem, public NesObject
{
public:
    Apu(bool isPal, IAudioProvider* audioProvider);
    virtual ~Apu();

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

    // Audio control
    void StartAudio(MemoryMap* cpuMemMap);
    void StopAudio();
    void PauseAudio();
    void UnpauseAudio();

    // Emulator interface
    virtual u8 loadb(u16 addr);
    virtual void storeb(u16 addr, u8 val);

    void Step(u32 &cycles, bool isDmaRunning, ApuStepResult& result);
    void Step(bool isDmaRunning, ApuStepResult& result, u32 &stealCycleCount);

    // SaveState / LoadState
    void SaveState(std::ofstream& ofs);
    void LoadState(std::ifstream& ifs);
private:

    // Registers
    u8 ReadApuStatus();
    void WriteApuStatus(u8 newStatus);

    void WriteApuPulse0(u8 val, ApuEnvelop* envelop, ApuPulseState* state);
    void WriteApuPulse1(u8 val, ApuPulseState* state);
    void WriteApuPulse2(u8 val, ApuPulseState* state);
    void WriteApuPulse3(u8 val, ApuEnvelop* envelop, ApuPulseState* state);

    void WriteApuTriangle0(u8 val);
    void WriteApuTriangle2(u8 val);
    void WriteApuTriangle3(u8 val);

    void WriteApuNoise0(u8 val);
    void WriteApuNoise2(u8 val);
    void WriteApuNoise3(u8 val);

    void WriteApuDmc0(u8 val);
    void WriteApuDmc1(u8 val);
    void WriteApuDmc2(u8 val);
    void WriteApuDmc3(u8 val);

    void WriteApuFrameCounter(u8 val);

    // Step control
    void ResetFrameCounter();
    void AdvanceFrameCounter(ApuStepResult& result);
    void DoQuarterFrameStep();
    void DoHalfFrameStep();
    
    void StepDmc(bool isDmaRunning, ApuStepResult& result, u32 &stealCycleCount);
    void LoadDmcSampleBuffer(bool isDmaRunning, ApuStepResult& result, u32 &stealCycleCount);
    void StepSweep(ApuPulseState* state);
    void StepPulseLengthCounter(ApuEnvelop* envelop, ApuPulseState* state);
    void StepTriangleLengthCounter();
    void StepNoiseLengthCounter();
    u32 StepEnvelop(ApuEnvelop* envelop); // Returns current volume

    // Utility
    void UpdateTriangle();
    void UpdatePulse(ApuPulseState* state);
    void UpdateNoise();
    void QueueAudioEvent(int setting, u32 newValue);
    u32 WavelengthToFrequency(bool isTriangle, int wavelength);

    // APU state information:
    NPtr<MemoryMap> _cpuMemMap;
    NPtr<AudioEngine> _audioEngine;
    bool _frameCounterMode1;
    bool _frameInterrupt;
    bool _frameInterruptInhibit;
    int _frameCycleCount;
    int _subframeCount;
    int _nextSubframeTimer;
    ApuPulseState* _pulseState1;
    ApuPulseState* _pulseState2;
    ApuTriangleState* _triangleState;
    ApuNoiseState* _noiseState;
    ApuDmcState* _dmcState;
    ApuEnvelop* _pulseEnvelop1;
    ApuEnvelop* _pulseEnvelop2;
    ApuEnvelop* _noiseEnvelop;
    u32 _lastTriangleFreq;

    // Emulator information:
    bool _isPal;
};