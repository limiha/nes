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

struct ApuPulseState
{
    int wavelength;
    int sweepPeriod;
    int sweepCounter;
    int shiftAmount;
    bool lengthDisabled;
    bool sweepEnabled;
    bool sweepReset;
    bool negate;
};

struct ApuTriangleState
{
    int wavelength;
    int counterReloadValue;
    bool lengthDisabled;
    bool haltCounter;
    bool reloadCounter;
};

struct ApuNoiseState
{
    int period;
    bool lengthDisabled;
    bool mode1;
    u16 shiftRegister;
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
};

// APU implementation

Apu::Apu(bool isPal)
    : _frameInterrupt(false)
    , _frameInterruptInhibit(false)
    , _frameCounterMode1(false)
    , _dmcInterrupt(false)
    , _frameCycleCount(0)
    , _frameCycleResetCounter(0)
    , _subframeCount(0)
    , _nextSubframeCycleCount(0)
    , _isPal(isPal)
{
    _pulseState1 = new ApuPulseState();
    _pulseState2 = new ApuPulseState();
    _triangleState = new ApuTriangleState();
    _noiseState = new ApuNoiseState();
    memset(_pulseState1, 0, sizeof(ApuPulseState));
    memset(_pulseState2, 0, sizeof(ApuPulseState));
    memset(_triangleState, 0, sizeof(ApuTriangleState));
    memset(_noiseState, 0, sizeof(ApuNoiseState));

    _pulseEnvelop1 = new ApuEnvelop();
    _pulseEnvelop2 = new ApuEnvelop();
    _noiseEnvelop = new ApuEnvelop();
    memset(_pulseEnvelop1, 0, sizeof(ApuEnvelop));
    memset(_pulseEnvelop2, 0, sizeof(ApuEnvelop));
    memset(_noiseEnvelop, 0, sizeof(ApuEnvelop));

    _pulse1 = new NesAudioPulseCtrl();
    _pulse2 = new NesAudioPulseCtrl();
    _triangle = new NesAudioTriangeCtrl();
    _noise = new NesAudioNoiseCtrl();
    _dmc = new NesAudioDmcCtrl();
    memset(_pulse1, 0, sizeof(NesAudioPulseCtrl));
    memset(_pulse2, 0, sizeof(NesAudioPulseCtrl));
    memset(_triangle, 0, sizeof(NesAudioTriangeCtrl));
    memset(_noise, 0, sizeof(NesAudioNoiseCtrl));
    memset(_dmc, 0, sizeof(NesAudioDmcCtrl));

    _audioEngine = new AudioEngine(_pulse1, _pulse2, _triangle, _noise, _dmc);
    _noiseState->shiftRegister = 1;
}

Apu::~Apu()
{
    delete _pulseState1;
    _pulseState1 = nullptr;

    delete _pulseState2;
    _pulseState2 = nullptr;

    delete _noiseState;
    _noiseState = nullptr;

    delete _pulse1;
    _pulse1 = nullptr;

    delete _pulse2;
    _pulse2 = nullptr;

    delete _triangle;
    _triangle = nullptr;

    delete _noise;
    _noise = nullptr;

    delete _dmc;
    _dmc = nullptr;
}

void Apu::StartAudio(int preferredSampleRate)
{
    int pulseMinFreq = WavelengthToFrequency(false, 0x07FF);
    int pulseMaxFreq = WavelengthToFrequency(false, 0x0008);
    int triangleMinFreq = WavelengthToFrequency(true, 0x07FF);
    int triangleMaxFreq = WavelengthToFrequency(true, 0x0010);

    _audioEngine->StartAudio(
        preferredSampleRate,
        pulseMinFreq,
        pulseMaxFreq,
        triangleMinFreq,
        triangleMaxFreq);
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
        WriteApuPulse0(val, _pulse1, _pulseEnvelop1);
        break;
    case 0x4001:
        WriteApuPulse1(val, _pulseState1);
        break;
    case 0x4002:
        WriteApuPulse2(val, _pulse1, _pulseState1);
        break;
    case 0x4003:
        WriteApuPulse3(val, _pulse1, _pulseEnvelop1, _pulseState1);
        break;
    case 0x4004:
        WriteApuPulse0(val, _pulse2, _pulseEnvelop2);
        break;
    case 0x4005:
        WriteApuPulse1(val, _pulseState2);
        break;
    case 0x4006:
        WriteApuPulse2(val, _pulse2, _pulseState2);
        break;
    case 0x4007:
        WriteApuPulse3(val, _pulse2, _pulseEnvelop2, _pulseState2);
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

void Apu::Load()
{
}

void Apu::Save()
{
}

void Apu::Step(u32 cycles, ApuStepResult& result)
{
    if (_frameCycleResetCounter != 0)
    {
        // Frame cycle counter reset is pending
        _frameCycleResetCounter -= cycles;
        if (_frameCycleResetCounter <= 0)
        {
            _frameCycleCount = -_frameCycleResetCounter;
            _frameCycleResetCounter = 0;

            // Reset the frame counter.
            ResetFrameCounter();
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

u8 Apu::ReadApuStatus()
{
    u8 status = 0;

    if (_pulse1->lengthCounter > 0)
        status |= 0x01;
    if (_pulse2->lengthCounter > 0)
        status |= 0x02;
    if (_triangle->lengthCounter > 0)
        status |= 0x04;
    if (_noise->lengthCounter > 0)
        status |= 0x08;
    if (_dmc->enabled)
        status |= 0x10;

    if (_frameInterrupt)
        status |= 0x40;
    if (_dmcInterrupt)
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
    _dmc->enabled = (newStatus & 0x10) != 0;

    if (_pulseState1->lengthDisabled)
        _pulse1->lengthCounter = 0;
    if (_pulseState2->lengthDisabled)
        _pulse2->lengthCounter = 0;
    if (_triangleState->lengthDisabled)
        _triangle->lengthCounter = 0;
    if (_noiseState->lengthDisabled)
        _noise->lengthCounter = 0;
}

void Apu::WriteApuPulse0(u8 val, NesAudioPulseCtrl* audioCtrl, ApuEnvelop* envelop)
{
    int dutyCycle = (val >> 6) & 0x03;
    bool haltCounter = (val & 0x20) != 0;
    bool constantVolumeFlag = (val & 0x10) != 0;
    int volumeOrDivider = val & 0x0F;

    envelop->haltCounter = haltCounter;
    envelop->setVolume = volumeOrDivider;
    envelop->envelopDivider = volumeOrDivider;
    envelop->constantVolume = constantVolumeFlag;
    audioCtrl->dutyCycle = dutyCycle;
}

void Apu::WriteApuPulse1(u8 val, ApuPulseState* state)
{
    state->sweepEnabled = (val & 0x80) != 0;
    state->negate = (val & 0x08) != 0;
    state->sweepPeriod = (val >> 4) & 0x07;
    state->shiftAmount = val & 0x07;
    state->sweepReset = true;
}

void Apu::WriteApuPulse2(u8 val, NesAudioPulseCtrl* audioCtrl, ApuPulseState* state)
{
    state->wavelength &= 0xFF00;
    state->wavelength |= val;

    if (state->wavelength < 8)
        audioCtrl->frequency = 0;
    else
        audioCtrl->frequency = WavelengthToFrequency(false, state->wavelength);
}

void Apu::WriteApuPulse3(u8 val, NesAudioPulseCtrl* audioCtrl, ApuEnvelop* envelop, ApuPulseState* state)
{
    if (!state->lengthDisabled)
        audioCtrl->lengthCounter = LengthCounterSetValues[val >> 3];

    state->wavelength &= 0x00FF;
    state->wavelength |= (val & 0x07) << 8;
    envelop->start = true;

    if (state->wavelength < 8)
        audioCtrl->frequency = 0;
    else
        audioCtrl->frequency = WavelengthToFrequency(false, state->wavelength);

    // Add this back after audio buffering is fixed
    //audioCtrl->phaseReset = true;
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
}

void Apu::WriteApuTriangle3(u8 val)
{
    if (!_triangleState->lengthDisabled)
        _triangle->lengthCounter = LengthCounterSetValues[val >> 3];

    _triangleState->wavelength &= 0x00FF;
    _triangleState->wavelength |= (val & 0x07) << 8;
    _triangleState->reloadCounter = true;

    if (_triangleState->wavelength < 2)
        _triangle->frequency = 0;
    else
        _triangle->frequency = WavelengthToFrequency(true, _triangleState->wavelength);
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
    _noiseState->mode1 = (val & 0x80) != 0;
    _noiseState->period = NoisePeriodValues[val & 0x0F];
}

void Apu::WriteApuNoise3(u8 val)
{
    if (!_noiseState->lengthDisabled)
        _noise->lengthCounter = LengthCounterSetValues[val >> 3];

    _noiseEnvelop->start = true;
}

void Apu::WriteApuDmc0(u8 val)
{
    // Not implemented
}

void Apu::WriteApuDmc1(u8 val)
{
    // Not implemented
}

void Apu::WriteApuDmc2(u8 val)
{
    // Not implemented
}

void Apu::WriteApuDmc3(u8 val)
{
    // Not implemented
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

        _frameCycleCount -= SubFrameCpuCycles_Mode0[SubFrameCountMode0 - 2];
        ResetFrameCounter();
    }
    else
    {
        // Mode 1

        DoHalfFrameStep();

        _frameCycleCount -= SubFrameCpuCycles_Mode1[SubFrameCountMode1 - 1];
        ResetFrameCounter();
    }
}

void Apu::DoQuarterFrameStep()
{
    _pulse1->volume = StepEnvelop(_pulseEnvelop1);
    _pulse2->volume = StepEnvelop(_pulseEnvelop2);
    _noise->volume = StepEnvelop(_noiseEnvelop);

    if (_triangleState->reloadCounter)
        _triangle->linearCounter = _triangleState->counterReloadValue;
    else if (_triangle->linearCounter != 0)
        _triangle->linearCounter--;

    if (!_triangleState->haltCounter)
        _triangleState->reloadCounter = false;
}

void Apu::DoHalfFrameStep()
{
    DoQuarterFrameStep();

    StepLengthCounter(_pulse1, _pulseEnvelop1);
    StepLengthCounter(_pulse2, _pulseEnvelop2);
    StepLengthCounter(_noise, _noiseEnvelop);
    StepLengthCounter(_triangle, _triangleState);
    StepSweep(_pulse1, _pulseState1, true);
    StepSweep(_pulse2, _pulseState2, false);
}

void Apu::StepNoise()
{
    u16 feedback = _noiseState->shiftRegister & 1;

    if (_noiseState->mode1)
        feedback ^= (_noiseState->shiftRegister & 0x0040) >> 6;
    else
        feedback ^= (_noiseState->shiftRegister & 0x0002) >> 1;

    _noiseState->shiftRegister >>= 1;
    _noiseState->shiftRegister |= feedback << 15;
    _noise->on = !(_noiseState->shiftRegister & 1);
}

void Apu::StepSweep(NesAudioPulseCtrl* audioCtrl, ApuPulseState* state, bool channel1)
{
    if (state->sweepEnabled)
    {
        if (state->sweepReset)
        {
            state->sweepCounter = 0;
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
                if (channel1)
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
            audioCtrl->frequency = WavelengthToFrequency(false, state->wavelength);
        }
    }
}


void Apu::StepLengthCounter(NesAudioPulseCtrl* audioCtrl, ApuEnvelop* envelop)
{
    if (!envelop->haltCounter && audioCtrl->lengthCounter != 0)
        audioCtrl->lengthCounter--;
}

void Apu::StepLengthCounter(NesAudioNoiseCtrl* audioCtrl, ApuEnvelop* envelop)
{
    if (!envelop->haltCounter && audioCtrl->lengthCounter != 0)
        audioCtrl->lengthCounter--;
}

void Apu::StepLengthCounter(NesAudioTriangeCtrl* audioCtrl, ApuTriangleState* state)
{
    if (!state->haltCounter && audioCtrl->lengthCounter != 0)
        audioCtrl->lengthCounter--;
}

int Apu::StepEnvelop(ApuEnvelop* envelop)
{
    if (envelop->start)
    {
        envelop->envelopVolume = 0x0F;
        envelop->dividerCounter = 0;
        envelop->start = false;
    }
    else if (!envelop->haltCounter)
    {
        if (envelop->dividerCounter++ == 0x0F)
        {
            envelop->dividerCounter = 0;
            if (envelop->envelopVolume == 0)
                envelop->envelopVolume = 0x0F;
            else
                envelop->envelopVolume--;
        }
    }

    if (!envelop->constantVolume)
        return envelop->envelopVolume;
    else
        return envelop->setVolume;
}
int Apu::WavelengthToFrequency(bool isTriangle, int wavelength)
{
    if (wavelength == 0)
        return 0;

    double steps = isTriangle ? TRIANGLE_WAVEFORM_STEPS : PULSE_WAVEFORM_STEPS;
    double cpuFreq = _isPal ? CPU_FREQ_PAL : CPU_FREQ_NTSC;
    return (int)(cpuFreq / (steps * (wavelength + 1)));
}
