#pragma once

#include "mem.h"

class AudioEngine;
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

class Apu : public IMem
{
public:
    Apu(bool isPal);
    ~Apu();

    // Audio control
    void StartAudio(MemoryMap* cpuMemMap, int preferredSampleRate);
    void StopAudio();
    void PauseAudio();
    void UnpauseAudio();

    // Emulator interface
    virtual u8 loadb(u16 addr);
    virtual void storeb(u16 addr, u8 val);

    void Step(u32 &cycles, bool isDmaRunning, ApuStepResult& result);

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
    
    void StepDmc(u32 &cycles, bool isDmaRunning, ApuStepResult& result);
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
    MemoryMap* _cpuMemMap;
    AudioEngine* _audioEngine;
    bool _frameCounterMode1;
    bool _frameInterrupt;
    bool _frameInterruptInhibit;
    int _frameCycleCount;
    int _frameCycleResetCounter;
    int _subframeCount;
    int _nextSubframeCycleCount;
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