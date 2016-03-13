#include "stdafx.h"
#include "audio.h"
#include "apu.h"

//#define APU_LOGGING

#define PULSE_WAVEFORM_STEPS 16
#define TRIANGLE_WAVEFORM_STEPS 32

// The number of CPU cycles per sub-frame in frame counter Mode 0
const int SubFrameCountMode0 = 5;
const int SubFrameCpuCycles_Mode0[SubFrameCountMode0] =
{
    7457, 14912, 22371, 29828, 29829,
};

// The number of CPU cycles per sub-frame in frame counter Mode 1
const int SubFrameCountMode1 = 4;
const int SubFrameCpuCycles_Mode1[SubFrameCountMode1] =
{
    7457, 14912, 22371, 37281
};

const int LengthCounterSetValues[32] =
{
    0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06, 0xA0, 0x08, 0x3C, 0x0A, 0x0E, 0x0C, 0x1A, 0x0E,
    0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16, 0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E
};

const int NoisePeriodValues[16] =
{
    0x01AC, 0x017C, 0x0154, 0x0140, 0x011E, 0x00FE, 0x00E2, 0x00D6,
    0x00BE, 0x00A0, 0x008E, 0x0080, 0x006A, 0x0054, 0x0048, 0x0036
};

const int DmcRateNTSC[16] =
{
    428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72, 54
};

const int DmcRatePAL[16] =
{
    398, 354, 316, 298, 276, 236, 210, 198, 176, 148, 132, 118, 98, 78, 66, 50
};

struct ApuPulseState
{
    int lengthCounter;
    int wavelength;
    int sweepPeriod;
    int sweepCounter;
    int shiftAmount;
    bool lengthDisabled;
    bool sweepEnabled;
    bool sweepReset;
    bool negate;
    u32 volume;

    // Last states written to audio engine
    // We don't save/load these because we don't directly save the audio engine state
    u32 lastFrequency;
    u32 lastVolume;

    // Constants
    int frequencySetting;
    int volumeSetting;
    int dutyCycleSetting;
    int phaseResetSetting;

    void SaveState(std::ofstream& ofs)
    {
        Util::WriteBytes(lengthCounter, ofs);
        Util::WriteBytes(wavelength, ofs);
        Util::WriteBytes(sweepPeriod, ofs);
        Util::WriteBytes(sweepCounter, ofs);
        Util::WriteBytes(shiftAmount, ofs);
        Util::WriteBytes(lengthDisabled, ofs);
        Util::WriteBytes(sweepEnabled, ofs);
        Util::WriteBytes(sweepReset, ofs);
        Util::WriteBytes(negate, ofs);
        Util::WriteBytes(volume, ofs);
    }

    void LoadState(std::ifstream& ifs)
    {
        Util::ReadBytes(lengthCounter, ifs);
        Util::ReadBytes(wavelength, ifs);
        Util::ReadBytes(sweepPeriod, ifs);
        Util::ReadBytes(sweepCounter, ifs);
        Util::ReadBytes(shiftAmount, ifs);
        Util::ReadBytes(lengthDisabled, ifs);
        Util::ReadBytes(sweepEnabled, ifs);
        Util::ReadBytes(sweepReset, ifs);
        Util::ReadBytes(negate, ifs);
        Util::ReadBytes(volume, ifs);
    }
};

struct ApuTriangleState
{
    int lengthCounter;
    int linearCounter;
    int wavelength;
    int counterReloadValue;
    bool lengthDisabled;
    bool haltCounter;
    bool reloadCounter;

    void SaveState(std::ofstream& ofs)
    {
        Util::WriteBytes(lengthCounter, ofs);
        Util::WriteBytes(linearCounter, ofs);
        Util::WriteBytes(wavelength, ofs);
        Util::WriteBytes(counterReloadValue, ofs);
        Util::WriteBytes(lengthDisabled, ofs);
        Util::WriteBytes(haltCounter, ofs);
        Util::WriteBytes(reloadCounter, ofs);
    }

    void LoadState(std::ifstream& ifs)
    {
        Util::ReadBytes(lengthCounter, ifs);
        Util::ReadBytes(linearCounter, ifs);
        Util::ReadBytes(wavelength, ifs);
        Util::ReadBytes(counterReloadValue, ifs);
        Util::ReadBytes(lengthDisabled, ifs);
        Util::ReadBytes(haltCounter, ifs);
        Util::ReadBytes(reloadCounter, ifs);
    }
};

struct ApuNoiseState
{
    int lengthCounter;
    bool lengthDisabled;
    u32 period;
    u32 volume;

    // Last states written to audio engine
    u32 lastPeriod;
    u32 lastVolume;

    void SaveState(std::ofstream& ofs)
    {
        Util::WriteBytes(lengthCounter, ofs);
        Util::WriteBytes(lengthDisabled, ofs);
        Util::WriteBytes(period, ofs);
        Util::WriteBytes(volume, ofs);
    }

    void LoadState(std::ifstream& ifs)
    {
        Util::ReadBytes(lengthCounter, ifs);
        Util::ReadBytes(lengthDisabled, ifs);
        Util::ReadBytes(period, ifs);
        Util::ReadBytes(volume, ifs);
    }
};

struct ApuDmcState
{
    bool enabled;
    bool interrupt;
    bool interruptEnabled;
    bool loop;
    bool bufferEmpty;
    u16 sampleAddress;
    u16 readAddress;
    int sampleSize;
    int bytesRemaining;
    int bitsRemaining;
    u32 cycleCount;
    u32 sampleRate;
    u8 sampleBuffer;
    u8 shiftRegister;
    u8 outputLevel;

    void SaveState(std::ofstream& ofs)
    {
        Util::WriteBytes(enabled, ofs);
        Util::WriteBytes(interrupt, ofs);
        Util::WriteBytes(interruptEnabled, ofs);
        Util::WriteBytes(loop, ofs);
        Util::WriteBytes(bufferEmpty, ofs);
        Util::WriteBytes(sampleAddress, ofs);
        Util::WriteBytes(readAddress, ofs);
        Util::WriteBytes(sampleSize, ofs);
        Util::WriteBytes(bytesRemaining, ofs);
        Util::WriteBytes(bitsRemaining, ofs);
        Util::WriteBytes(cycleCount, ofs);
        Util::WriteBytes(sampleRate, ofs);
        Util::WriteBytes(sampleBuffer, ofs);
        Util::WriteBytes(shiftRegister, ofs);
        Util::WriteBytes(outputLevel, ofs);
    }

    void LoadState(std::ifstream& ifs)
    {
        Util::ReadBytes(enabled, ifs);
        Util::ReadBytes(interrupt, ifs);
        Util::ReadBytes(interruptEnabled, ifs);
        Util::ReadBytes(loop, ifs);
        Util::ReadBytes(bufferEmpty, ifs);
        Util::ReadBytes(sampleAddress, ifs);
        Util::ReadBytes(readAddress, ifs);
        Util::ReadBytes(sampleSize, ifs);
        Util::ReadBytes(bytesRemaining, ifs);
        Util::ReadBytes(bitsRemaining, ifs);
        Util::ReadBytes(cycleCount, ifs);
        Util::ReadBytes(sampleRate, ifs);
        Util::ReadBytes(sampleBuffer, ifs);
        Util::ReadBytes(shiftRegister, ifs);
        Util::ReadBytes(outputLevel, ifs);
    }
};

struct ApuEnvelop
{
    int envelopDivider;
    int dividerCounter;
    int setVolume;
    int envelopVolume;
    bool start;
    bool haltCounter;
    bool constantVolume;

    void SaveState(std::ofstream& ofs)
    {
        Util::WriteBytes(envelopDivider, ofs);
        Util::WriteBytes(dividerCounter, ofs);
        Util::WriteBytes(setVolume, ofs);
        Util::WriteBytes(envelopVolume, ofs);
        Util::WriteBytes(start, ofs);
        Util::WriteBytes(haltCounter, ofs);
        Util::WriteBytes(constantVolume, ofs);
    }

    void LoadState(std::ifstream& ifs)
    {
        Util::ReadBytes(envelopDivider, ifs);
        Util::ReadBytes(dividerCounter, ifs);
        Util::ReadBytes(setVolume, ifs);
        Util::ReadBytes(envelopVolume, ifs);
        Util::ReadBytes(start, ifs);
        Util::ReadBytes(haltCounter, ifs);
        Util::ReadBytes(constantVolume, ifs);
    }
};

// APU implementation

Apu::Apu(bool isPal)
    : _frameInterrupt(false)
    , _frameInterruptInhibit(false)
    , _frameCounterMode1(false)
    , _frameCycleCount(0)
    , _frameCycleResetCounter(0)
    , _subframeCount(0)
    , _nextSubframeCycleCount(0)
    , _isPal(isPal)
    , _lastTriangleFreq(0)
{
    _pulseState1 = new ApuPulseState();
    _pulseState2 = new ApuPulseState();
    _triangleState = new ApuTriangleState();
    _noiseState = new ApuNoiseState();
    _dmcState = new ApuDmcState();
    memset(_pulseState1, 0, sizeof(ApuPulseState));
    memset(_pulseState2, 0, sizeof(ApuPulseState));
    memset(_triangleState, 0, sizeof(ApuTriangleState));
    memset(_noiseState, 0, sizeof(ApuNoiseState));
    memset(_dmcState, 0, sizeof(ApuDmcState));

    _pulseEnvelop1 = new ApuEnvelop();
    _pulseEnvelop2 = new ApuEnvelop();
    _noiseEnvelop = new ApuEnvelop();
    memset(_pulseEnvelop1, 0, sizeof(ApuEnvelop));
    memset(_pulseEnvelop2, 0, sizeof(ApuEnvelop));
    memset(_noiseEnvelop, 0, sizeof(ApuEnvelop));

    _audioEngine = new AudioEngine();
    _pulseState1->frequencySetting = NESAUDIO_PULSE1_FREQUENCY;
    _pulseState2->frequencySetting = NESAUDIO_PULSE2_FREQUENCY;
    _pulseState1->dutyCycleSetting = NESAUDIO_PULSE1_DUTYCYCLE;
    _pulseState2->dutyCycleSetting = NESAUDIO_PULSE2_DUTYCYCLE;
    _pulseState1->volumeSetting = NESAUDIO_PULSE1_VOLUME;
    _pulseState2->volumeSetting = NESAUDIO_PULSE2_VOLUME;
    _pulseState1->phaseResetSetting = NESAUDIO_PULSE1_PHASE_RESET;
    _pulseState2->phaseResetSetting = NESAUDIO_PULSE2_PHASE_RESET;
    _dmcState->bufferEmpty = true;
}

Apu::~Apu()
{
    delete _pulseState1;
    _pulseState1 = nullptr;

    delete _pulseState2;
    _pulseState2 = nullptr;

    delete _noiseState;
    _noiseState = nullptr;
}

void Apu::StartAudio(MemoryMap* cpuMemMap, int preferredSampleRate)
{
    int pulseMinFreq = WavelengthToFrequency(false, 0x07FF);
    int pulseMaxFreq = WavelengthToFrequency(false, 0x0008);
    int triangleMinFreq = WavelengthToFrequency(true, 0x07FF);
    int triangleMaxFreq = WavelengthToFrequency(true, 0x0010);

    _audioEngine->StartAudio(
        preferredSampleRate,
        _isPal ? CPU_FREQ_PAL : CPU_FREQ_NTSC,
        pulseMinFreq,
        pulseMaxFreq,
        triangleMinFreq,
        triangleMaxFreq);

    _cpuMemMap = cpuMemMap;
}

void Apu::StopAudio()
{
    _audioEngine->StopAudio();
}

void Apu::PauseAudio()
{
    _audioEngine->PauseAudio();
}

void Apu::UnpauseAudio()
{
    _audioEngine->UnpauseAudio();
}

u8 Apu::loadb(u16 addr)
{
    if (addr == 0x4015)
    {
        // $4015 (APU Status) is the only register that can be read
        u8 status = ReadApuStatus();

#if defined(APU_LOGGING)
        printf_s("Read  $4015: status -> $%02x\n", status);
#endif

        return status;
    }

    return 0;
}

void Apu::storeb(u16 addr, u8 val)
{
#if defined(APU_LOGGING)
    printf_s("Write $%04x: $%02x\n", addr, val);
#endif

    switch (addr)
    {
    case 0x4000:
        WriteApuPulse0(val, _pulseEnvelop1, _pulseState1);
        break;
    case 0x4001:
        WriteApuPulse1(val, _pulseState1);
        break;
    case 0x4002:
        WriteApuPulse2(val, _pulseState1);
        break;
    case 0x4003:
        WriteApuPulse3(val, _pulseEnvelop1, _pulseState1);
        break;
    case 0x4004:
        WriteApuPulse0(val, _pulseEnvelop2, _pulseState2);
        break;
    case 0x4005:
        WriteApuPulse1(val, _pulseState2);
        break;
    case 0x4006:
        WriteApuPulse2(val, _pulseState2);
        break;
    case 0x4007:
        WriteApuPulse3(val, _pulseEnvelop2, _pulseState2);
        break;
    case 0x4008:
        WriteApuTriangle0(val);
        break;
    case 0x400A:
        WriteApuTriangle2(val);
        break;
    case 0x400B:
        WriteApuTriangle3(val);
        break;
    case 0x400C:
        WriteApuNoise0(val);
        break;
    case 0x400E:
        WriteApuNoise2(val);
        break;
    case 0x400F:
        WriteApuNoise3(val);
        break;
    case 0x4010:
        WriteApuDmc0(val);
        break;
    case 0x4011:
        WriteApuDmc1(val);
        break;
    case 0x4012:
        WriteApuDmc2(val);
        break;
    case 0x4013:
        WriteApuDmc3(val);
        break;
    case 0x4015:
        WriteApuStatus(val);
        break;
    case 0x4017:
        WriteApuFrameCounter(val);
        break;
    }
}

void Apu::Step(u32 &cycles, bool isDmaRunning, ApuStepResult& result)
{
    if (_dmcState->enabled)
        StepDmc(cycles, isDmaRunning, result);

    if (_frameCycleResetCounter != 0)
    {
        // Frame cycle counter reset is pending
        _frameCycleResetCounter -= cycles;
        if (_frameCycleResetCounter <= 0)
        {
            ResetFrameCounter();

            _frameCycleCount = -_frameCycleResetCounter;
            _frameCycleResetCounter = 0;
        }
        else
        {
            _frameCycleCount += cycles;
        }
    }
    else
    {
        _frameCycleCount += cycles;
    }
    
    if (_frameCycleCount < _nextSubframeCycleCount)
    {
        // Don't need to move to the next internal step yet
        return;
    }

    // Advance to the next frame
    AdvanceFrameCounter(result);
}

void Apu::SaveState(std::ofstream& ofs)
{
    PauseAudio();

    Util::WriteBytes(_frameCounterMode1, ofs);
    Util::WriteBytes(_frameInterrupt, ofs);
    Util::WriteBytes(_frameInterruptInhibit, ofs);
    Util::WriteBytes(_frameCycleCount, ofs);
    Util::WriteBytes(_frameCycleResetCounter, ofs);
    Util::WriteBytes(_subframeCount, ofs);
    Util::WriteBytes(_nextSubframeCycleCount, ofs);
    Util::WriteBytes(_lastTriangleFreq, ofs);

    _pulseState1->SaveState(ofs);
    _pulseState2->SaveState(ofs);
    _triangleState->SaveState(ofs);
    _dmcState->SaveState(ofs);
    _pulseEnvelop1->SaveState(ofs);
    _pulseEnvelop2->SaveState(ofs);

    UnpauseAudio();
}

void Apu::LoadState(std::ifstream& ifs)
{
    PauseAudio();

    Util::ReadBytes(_frameCounterMode1, ifs);
    Util::ReadBytes(_frameInterrupt, ifs);
    Util::ReadBytes(_frameInterruptInhibit, ifs);
    Util::ReadBytes(_frameCycleCount, ifs);
    Util::ReadBytes(_frameCycleResetCounter, ifs);
    Util::ReadBytes(_subframeCount, ifs);
    Util::ReadBytes(_nextSubframeCycleCount, ifs);
    Util::ReadBytes(_lastTriangleFreq, ifs);

    _pulseState1->LoadState(ifs);
    _pulseState2->LoadState(ifs);
    _triangleState->LoadState(ifs);
    _dmcState->LoadState(ifs);
    _pulseEnvelop1->LoadState(ifs);
    _pulseEnvelop2->LoadState(ifs);

    // Force frame reset and Send loaded settings to audio engine
    QueueAudioEvent(NESAUDIO_FRAME_RESET, 0);
    QueueAudioEvent(NESAUDIO_DMC_VALUE, _dmcState->outputLevel);
    QueueAudioEvent(NESAUDIO_PULSE1_DUTYCYCLE, _pulseState1->dutyCycleSetting);
    QueueAudioEvent(NESAUDIO_PULSE2_DUTYCYCLE, _pulseState2->dutyCycleSetting);
    UpdatePulse(_pulseState1);
    UpdatePulse(_pulseState2);
    UpdateTriangle();
    UpdateNoise();

    UnpauseAudio();
}

u8 Apu::ReadApuStatus()
{
    u8 status = 0;

    if (_pulseState1->lengthCounter > 0)
        status |= 0x01;
    if (_pulseState2->lengthCounter > 0)
        status |= 0x02;
    if (_triangleState->lengthCounter > 0)
        status |= 0x04;
    if (_noiseState->lengthCounter > 0)
        status |= 0x08;
    if (_dmcState->bytesRemaining > 0)
        status |= 0x10;

    if (_frameInterrupt)
        status |= 0x40;
    if (_dmcState->interrupt)
        status |= 0x80;

    _frameInterrupt = false;

    return status;
}

void Apu::WriteApuStatus(u8 newStatus)
{
    _pulseState1->lengthDisabled = (newStatus & 0x01) == 0;
    _pulseState2->lengthDisabled = (newStatus & 0x02) == 0;
    _triangleState->lengthDisabled = (newStatus & 0x04) == 0;
    _noiseState->lengthDisabled = (newStatus & 0x08) == 0;
    bool enableDmc = (newStatus & 0x10) != 0;

    if (enableDmc != _dmcState->enabled)
    {
        if (enableDmc)
        {
            _dmcState->readAddress = _dmcState->sampleAddress;
            _dmcState->bytesRemaining = _dmcState->sampleSize;
            _dmcState->enabled = _dmcState->sampleRate > 0 && _dmcState->bytesRemaining > 0;
        }
        else
        {
            _dmcState->enabled = false;
            _dmcState->bytesRemaining = 0;
        }
    }

    if (_pulseState1->lengthDisabled)
    {
        _pulseState1->lengthCounter = 0;
        UpdatePulse(_pulseState1);
    }

    if (_pulseState2->lengthDisabled)
    {
        _pulseState2->lengthCounter = 0;
        UpdatePulse(_pulseState2);
    }

    if (_triangleState->lengthDisabled)
    {
        _triangleState->lengthCounter = 0;
        UpdateTriangle();
    }

    if (_noiseState->lengthDisabled)
    {
        _noiseState->lengthCounter = 0;
        UpdateNoise();
    }
}

void Apu::WriteApuPulse0(u8 val, ApuEnvelop* envelop, ApuPulseState* state)
{
    int dutyCycle = (val >> 6) & 0x03;
    bool haltCounter = (val & 0x20) != 0;
    bool constantVolumeFlag = (val & 0x10) != 0;
    int volumeOrDivider = val & 0x0F;

    envelop->haltCounter = haltCounter;
    envelop->setVolume = volumeOrDivider;
    envelop->envelopDivider = volumeOrDivider;
    envelop->constantVolume = constantVolumeFlag;

    QueueAudioEvent(state->dutyCycleSetting, dutyCycle);
}

void Apu::WriteApuPulse1(u8 val, ApuPulseState* state)
{
    state->sweepEnabled = (val & 0x80) != 0;
    state->negate = (val & 0x08) != 0;
    state->sweepPeriod = ((val >> 4) & 0x07) + 1;
    state->shiftAmount = val & 0x07;
    state->sweepReset = true;
}

void Apu::WriteApuPulse2(u8 val, ApuPulseState* state)
{
    state->wavelength &= 0xFF00;
    state->wavelength |= val;

    UpdatePulse(state);
}

void Apu::WriteApuPulse3(u8 val, ApuEnvelop* envelop, ApuPulseState* state)
{
    state->wavelength &= 0x00FF;
    state->wavelength |= (val & 0x07) << 8;
    envelop->start = true;

    if (!state->lengthDisabled)
    {
        state->lengthCounter = LengthCounterSetValues[val >> 3];
        UpdatePulse(state);
        QueueAudioEvent(state->phaseResetSetting, 0);
    }
}

void Apu::WriteApuTriangle0(u8 val)
{
    _triangleState->haltCounter = (val & 0x80) != 0;
    _triangleState->counterReloadValue = val & 0x7F;
}

void Apu::WriteApuTriangle2(u8 val)
{
    _triangleState->wavelength &= 0xFF00;
    _triangleState->wavelength |= val;

    if (!_triangleState->lengthDisabled)
        UpdateTriangle();
}

void Apu::WriteApuTriangle3(u8 val)
{
    _triangleState->wavelength &= 0x00FF;
    _triangleState->wavelength |= (val & 0x07) << 8;
    _triangleState->reloadCounter = true;

    if (!_triangleState->lengthDisabled)
    {
        _triangleState->lengthCounter = LengthCounterSetValues[val >> 3];
        UpdateTriangle();
    }
}

void Apu::WriteApuNoise0(u8 val)
{
    bool haltCounter = (val & 0x20) != 0;
    bool constantVolumeFlag = (val & 0x10) != 0;
    int volumeOrDivider = val & 0x0F;

    _noiseEnvelop->haltCounter = haltCounter;
    _noiseEnvelop->setVolume = volumeOrDivider;
    _noiseEnvelop->envelopDivider = volumeOrDivider;
    _noiseEnvelop->constantVolume = constantVolumeFlag;
}

void Apu::WriteApuNoise2(u8 val)
{
    u32 modeSetting = val >> 7;
    QueueAudioEvent(NESAUDIO_NOISE_MODE, modeSetting);

    _noiseState->period = NoisePeriodValues[val & 0x0F];

    UpdateNoise();
}

void Apu::WriteApuNoise3(u8 val)
{
    if (!_noiseState->lengthDisabled)
    {
        _noiseState->lengthCounter = LengthCounterSetValues[val >> 3];
        UpdateNoise();
    }

    _noiseEnvelop->start = true;
}

void Apu::WriteApuDmc0(u8 val)
{
    if ((val & 0x80) == 0)
    {
        _dmcState->interrupt = false;
        _dmcState->interruptEnabled = false;
    }
    else
    {
        _dmcState->interruptEnabled = true;
    }

    _dmcState->loop = (val & 0x40) != 0;

    int rateIndex = val & 0x0F;
    if (_isPal)
        _dmcState->sampleRate = DmcRatePAL[rateIndex];
    else
        _dmcState->sampleRate = DmcRateNTSC[rateIndex];
}

void Apu::WriteApuDmc1(u8 val)
{
    _dmcState->outputLevel = val & 0x7F;
    QueueAudioEvent(NESAUDIO_DMC_VALUE, _dmcState->outputLevel);
}

void Apu::WriteApuDmc2(u8 val)
{
    _dmcState->sampleAddress = val * 64 + 0xC000;
}

void Apu::WriteApuDmc3(u8 val)
{
    _dmcState->sampleSize = val * 16 + 1;
}

void Apu::WriteApuFrameCounter(u8 val)
{
    if (val & 0x40)
    {
        // Interrupt inhibit
        _frameInterruptInhibit = true;
        _frameInterrupt = false;
    }
    else
    {
        _frameInterruptInhibit = false;
    }

    _frameCounterMode1 = (val & 0x80) != 0;
    if (_frameCounterMode1)
    {
        // Setting the frame counter mode 1 bit immediately clocks the half frame units
        DoHalfFrameStep();
    }

    // We need to reset the frame counter, but it doesn't take effect immediately so set
    // a count-down timer for reseting the frame counter.
    // Count-down time is set to:
    //     3 clock cycles if the write occured on an even clock cycle
    //     4 clock cycles if the write occured on an odd clock cycle
    _frameCycleResetCounter = 3 + (_frameCycleCount & 1);
}

void Apu::ResetFrameCounter()
{
    // Tell the audio engine
    QueueAudioEvent(NESAUDIO_FRAME_RESET, 0);

    _subframeCount = 0;

    // Get the next cycle number to advance on.
    // Doesn't matter which mode we use because the first 3 indices are the same.
    _nextSubframeCycleCount = SubFrameCpuCycles_Mode0[0];
}

void Apu::AdvanceFrameCounter(ApuStepResult& result)
{
    _subframeCount++;

    // Determine when we need to advance to the next frame
    if (_frameCounterMode1)
        _nextSubframeCycleCount = SubFrameCpuCycles_Mode1[_subframeCount];
    else
        _nextSubframeCycleCount = SubFrameCpuCycles_Mode0[_subframeCount];

    // The first 3 frames are the same for both mode 0 and 1
    switch (_subframeCount)
    {
    case 1:
        DoQuarterFrameStep();
        return;
    case 2:
        DoHalfFrameStep();
        return;
    case 3:
        DoQuarterFrameStep();
        return;
    }

    if (!_frameCounterMode1)
    {
        // Mode 0

        if (!_frameInterruptInhibit)
        {
            // Generate IRQ
            if (!_frameInterrupt)
                result.Irq = true;

            _frameInterrupt = true;
        }

        DoHalfFrameStep();

        ResetFrameCounter();
        _frameCycleCount -= SubFrameCpuCycles_Mode0[SubFrameCountMode0 - 2];
    }
    else
    {
        // Mode 1

        DoHalfFrameStep();

        ResetFrameCounter();
        _frameCycleCount -= SubFrameCpuCycles_Mode1[SubFrameCountMode1 - 1];
    }
}

void Apu::DoQuarterFrameStep()
{
    _pulseState1->volume = StepEnvelop(_pulseEnvelop1);
    _pulseState2->volume = StepEnvelop(_pulseEnvelop2);
    _noiseState->volume = StepEnvelop(_noiseEnvelop);

    UpdatePulse(_pulseState1);
    UpdatePulse(_pulseState2);
    UpdateNoise();

    if (_triangleState->reloadCounter)
    {
        _triangleState->linearCounter = _triangleState->counterReloadValue;
        UpdateTriangle();
    }
    else if (_triangleState->linearCounter != 0)
    {
        if (--_triangleState->linearCounter == 0)
            UpdateTriangle();
    }

    if (!_triangleState->haltCounter)
        _triangleState->reloadCounter = false;
}

void Apu::DoHalfFrameStep()
{
    DoQuarterFrameStep();

    StepPulseLengthCounter(_pulseEnvelop1, _pulseState1);
    StepPulseLengthCounter(_pulseEnvelop2, _pulseState2);
    StepNoiseLengthCounter();
    StepTriangleLengthCounter();
    StepSweep(_pulseState1);
    StepSweep(_pulseState2);
}

void Apu::StepDmc(u32 &cycles, bool isDmaRunning, ApuStepResult& result)
{
    _dmcState->cycleCount += cycles;
    if (_dmcState->cycleCount < _dmcState->sampleRate)
    {
        // DMC doesn't have any work to do yet.
        return;
    }

    _dmcState->cycleCount -= _dmcState->sampleRate;

    if (_dmcState->bufferEmpty)
    {
        if (_dmcState->bytesRemaining == 0)
        {
            if (_dmcState->loop)
            {
                // Loop back around again
                _dmcState->readAddress = _dmcState->sampleAddress;
                _dmcState->bytesRemaining = _dmcState->sampleSize;
            }
            else if (_dmcState->interruptEnabled)
            {
                // Looping disabled.  Set IRQ flag.
                _dmcState->interrupt = true;
                result.Irq = true;
            }
        }
        
        if (_dmcState->bytesRemaining != 0)
        {
            // Read the next sample into the buffer.
            _dmcState->sampleBuffer = _cpuMemMap->loadb(_dmcState->readAddress);

            _dmcState->readAddress++;
            if (_dmcState->readAddress == 0)
                _dmcState->readAddress = 0x8000; // Read address wraps from $FFFF to $8000

            _dmcState->bytesRemaining--;
            _dmcState->bufferEmpty = false;

            // The CPU is paused while the DMC reads memory.
            // We need to account for the time taken by incrementing the cycle count
            // TODO: Accurate cycle counting when CPU is executing a write instruction
            if (isDmaRunning)
                cycles += 2; // Read takes 2 clock cycles if DMA is running
            else
                cycles += 4; // Read takes 4 clock cycles unless CPU is doing a write instruction - then 3 (TODO)
        }
    }

    if (_dmcState->bitsRemaining == 0 && !_dmcState->bufferEmpty)
    {
        _dmcState->shiftRegister = _dmcState->sampleBuffer;
        _dmcState->bufferEmpty = true;
        _dmcState->bitsRemaining = 8;
    }
    
    if (_dmcState->bitsRemaining != 0)
    {
        _dmcState->bitsRemaining--;

        u8 bit0 = _dmcState->shiftRegister & 1;
        _dmcState->shiftRegister >>= 1;

        u8 outputLevel = _dmcState->outputLevel;
        if (bit0)
            outputLevel += 2;
        else
            outputLevel -= 2;

        // Only change output level if it doesn't underflow/overflow the 0-127 range
        if ((outputLevel & 0x80) == 0)
        {
            _dmcState->outputLevel = outputLevel;
            QueueAudioEvent(NESAUDIO_DMC_VALUE, outputLevel);
        }
    }
}

void Apu::StepSweep(ApuPulseState* state)
{
    if (state->sweepEnabled)
    {
        if (state->sweepReset)
        {
            state->sweepCounter = state->sweepPeriod;
            state->sweepReset = false;
        }
        else if (state->sweepCounter != 0)
        {
            state->sweepCounter--;
        }
        else
        {
            int wavelength = state->wavelength;
            int delta = wavelength >> state->shiftAmount;
            if (state->negate)
            {
                wavelength -= delta;

                // Add an extra decrement for channel 1
                if (state->frequencySetting == NESAUDIO_PULSE1_FREQUENCY)
                    wavelength--;
            }
            else
            {
                wavelength += delta;
            }

            if (wavelength < 8 || wavelength > 0x07FF)
                wavelength = 0;

            state->wavelength = wavelength;
            state->sweepCounter = state->sweepPeriod;
            UpdatePulse(state);
        }
    }
}

void Apu::StepPulseLengthCounter(ApuEnvelop* envelop, ApuPulseState* state)
{
    if (!envelop->haltCounter && state->lengthCounter != 0)
    {
        if (--state->lengthCounter == 0)
            QueueAudioEvent(state->frequencySetting, 0);
    }
}

void Apu::StepTriangleLengthCounter()
{
    if (!_triangleState->haltCounter && _triangleState->lengthCounter != 0)
    {
        if (--_triangleState->lengthCounter == 0)
            UpdateTriangle();
    }
}

void Apu::StepNoiseLengthCounter()
{
    if (!_noiseEnvelop->haltCounter && _noiseState->lengthCounter != 0)
    {
        if (--_noiseState->lengthCounter == 0)
            UpdateNoise();
    }
}

u32 Apu::StepEnvelop(ApuEnvelop* envelop)
{
    if (envelop->start)
    {
        envelop->envelopVolume = 0x0F;
        envelop->dividerCounter = envelop->envelopDivider;
        envelop->start = false;
    }
    else
    {
        if (envelop->dividerCounter != 0)
        {
            envelop->dividerCounter--;
        }
        else
        {
            envelop->dividerCounter = envelop->envelopDivider;
            if (envelop->envelopVolume == 0)
            {
                if (envelop->haltCounter)
                    envelop->envelopVolume = 0x0F;
            }
            else
            {
                envelop->envelopVolume--;
            }
        }
    }

    if (!envelop->constantVolume)
        return envelop->envelopVolume;
    else
        return envelop->setVolume;
}

void Apu::UpdateTriangle()
{
    u32 freq;
    if (_triangleState->wavelength > 2 && _triangleState->lengthCounter > 0 && _triangleState->linearCounter > 0)
        freq = WavelengthToFrequency(true, _triangleState->wavelength);
    else
        freq = 0;

    if (freq != _lastTriangleFreq)
        QueueAudioEvent(NESAUDIO_TRIANGLE_FREQUENCY, freq);

    _lastTriangleFreq = freq;
}

void Apu::UpdatePulse(ApuPulseState* state)
{
    u32 freq;
    if (state->wavelength > 2 && state->lengthCounter > 0)
        freq = WavelengthToFrequency(false, state->wavelength);
    else
        freq = 0;

    if (freq != state->lastFrequency)
        QueueAudioEvent(state->frequencySetting, freq);

    if (state->volume != state->lastVolume)
        QueueAudioEvent(state->volumeSetting, state->volume);

    state->lastFrequency = freq;
    state->lastVolume = state->volume;
}

void Apu::UpdateNoise()
{
    u32 period = _noiseState->lengthCounter != 0 ? _noiseState->period : 0;

    if (_noiseState->lastPeriod != period)
        QueueAudioEvent(NESAUDIO_NOISE_PERIOD, period);

    if (_noiseState->lastVolume != _noiseState->volume)
        QueueAudioEvent(NESAUDIO_NOISE_VOLUME, _noiseState->volume);

    _noiseState->lastPeriod = period;
    _noiseState->lastVolume = _noiseState->volume;
}

void Apu::QueueAudioEvent(int setting, u32 newValue)
{
    _audioEngine->QueueAudioEvent(_frameCycleCount, setting, newValue);
}

u32 Apu::WavelengthToFrequency(bool isTriangle, int wavelength)
{
    if (wavelength == 0)
        return 0;

    double steps = isTriangle ? TRIANGLE_WAVEFORM_STEPS : PULSE_WAVEFORM_STEPS;
    double cpuFreq = _isPal ? CPU_FREQ_PAL : CPU_FREQ_NTSC;
    return (u32)(cpuFreq / (steps * (wavelength + 1)));
}
