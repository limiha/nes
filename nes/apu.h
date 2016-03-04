#pragma once

#include "mem.h"

class AudioEngine;
struct ApuPulseState;
struct ApuTriangleState;
struct ApuNoiseState;
struct ApuEnvelop;
struct NesAudioPulseCtrl;
struct NesAudioTriangeCtrl;
struct NesAudioNoiseCtrl;
struct NesAudioDmcCtrl;

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
    void StartAudio(int preferredSampleRate);
    void StopAudio();
    void PauseAudio();
    void UnpauseAudio();

    // Emulator interface
    virtual u8 loadb(u16 addr);
    virtual void storeb(u16 addr, u8 val);

    void Step(u32 cycles, ApuStepResult& result);
private:

    // Registers
    u8 ReadApuStatus();
    void WriteApuStatus(u8 newStatus);

    void WriteApuPulse0(u8 val, NesAudioPulseCtrl* audioCtrl, ApuEnvelop* envelop);
    void WriteApuPulse1(u8 val, ApuPulseState* pState);
    void WriteApuPulse2(u8 val, NesAudioPulseCtrl* audioCtrl, ApuPulseState* state);
    void WriteApuPulse3(u8 val, NesAudioPulseCtrl* audioCtrl, ApuEnvelop* envelop, ApuPulseState* state);

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
    void DoQuarterFrameStep();
    void DoHalfFrameStep();
    
    void StepNoise();
    void StepSweep(NesAudioPulseCtrl* audioCtrl, ApuPulseState* state, bool channel1);
    static void StepLengthCounter(NesAudioPulseCtrl* audioCtrl, ApuEnvelop* envelop);
    static void StepLengthCounter(NesAudioNoiseCtrl* audioCtrl, ApuEnvelop* envelop);
    static void StepLengthCounter(NesAudioTriangeCtrl* audioCtrl, ApuTriangleState* state);
    static int StepEnvelop(ApuEnvelop* envelop); // Returns current volume

    // Utility
    int WavelengthToFrequency(bool isTriangle, int wavelength);

    // APU state information:
    AudioEngine* _audioEngine;
    bool _counterEnabledFlag;
    bool _frameCounterMode1;
    bool _frameInterrupt;
    bool _frameInterruptInhibit;
    bool _dmcInterrupt;
    int _frameCycleCount;
    int _noiseCycleCount;
    int _frameNum;
    int _lengthCounterCode; // The value read/written via the APU registers.  Actual length is read from a lookup table.
    ApuPulseState* _pulseState1;
    ApuPulseState* _pulseState2;
    ApuTriangleState* _triangleState;
    ApuNoiseState* _noiseState;
    ApuEnvelop* _pulseEnvelop1;
    ApuEnvelop* _pulseEnvelop2;
    ApuEnvelop* _noiseEnvelop;

    // Emulator information:
    bool _isPal;

    // Channel Information:
    //
    // These are used to control the sound channels in real time.
    // Information in these structs is modified by Apu on the emulator thread and read by AudioEngine on the audio callback thread.
    NesAudioPulseCtrl* _pulse1;
    NesAudioPulseCtrl* _pulse2;
    NesAudioTriangeCtrl* _triangle;
    NesAudioNoiseCtrl* _noise;
    NesAudioDmcCtrl* _dmc;
};