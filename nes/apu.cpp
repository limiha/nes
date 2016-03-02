#include "stdafx.h"
#include "audio.h"
#include "apu.h"

#define PULSE_WAVEFORM_STEPS 16
#define TRIANGLE_WAVEFORM_STEPS 32

struct ApuPulseState
{
    int wavelength;
    bool halt;
    bool constantVolume;
};

// APU implementation

Apu::Apu(bool isPal)
    : _cycleCount(0)
    , _counterEnabledFlag(false)
    , _frameInterrupt(false)
    , _dmcInterrupt(false)
    , _lengthCounterCode(0)
    , _triangeWavelength(0)
    , _isPal(isPal)
{
    _pulseState1 = new ApuPulseState();
    _pulseState2 = new ApuPulseState();
    memset(_pulseState1, 0, sizeof(ApuPulseState));
    memset(_pulseState2, 0, sizeof(ApuPulseState));

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
}

Apu::~Apu()
{
    delete _pulseState1;
    _pulseState1 = nullptr;

    delete _pulseState2;
    _pulseState2 = nullptr;

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
        return ReadApuStatus();
    }

    return 0;
}

void Apu::storeb(u16 addr, u8 val)
{
    switch (addr)
    {
    case 0x4000:
        WriteApuPulse0(val, _pulse1, _pulseState1);
        break;
    case 0x4001:
        WriteApuPulse1(val, _pulseState1);
        break;
    case 0x4002:
        WriteApuPulse2(val, _pulse1, _pulseState1);
        break;
    case 0x4003:
        WriteApuPulse3(val, _pulse1, _pulseState1);
        break;
    case 0x4004:
        WriteApuPulse0(val, _pulse2, _pulseState2);
        break;
    case 0x4005:
        WriteApuPulse1(val, _pulseState2);
        break;
    case 0x4006:
        WriteApuPulse2(val, _pulse2, _pulseState2);
        break;
    case 0x4007:
        WriteApuPulse3(val, _pulse2, _pulseState2);
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

void Apu::Step(u32 cycles, ApuStepResult& result)
{
    // Dynamic state isn't implemented yet
}

u8 Apu::ReadApuStatus()
{
    u8 status = 0;

    if (_pulse1->enabled)
        status |= 1;
    if (_pulse2->enabled)
        status |= 2;
    if (_triangle->enabled)
        status |= 4;
    if (_noise->enabled)
        status |= 8;
    if (_dmc->enabled)
        status |= 16;

    if (_frameInterrupt)
        status |= 64;
    if (_dmcInterrupt)
        status |= 128;

    _frameInterrupt = false;

    return status;
}

void Apu::WriteApuStatus(u8 newStatus)
{
    _pulse1->enabled = (newStatus & 1) != 0;
    _pulse2->enabled = (newStatus & 2) != 0;
    _triangle->enabled = (newStatus & 4) != 0;
    _noise->enabled = (newStatus & 8) != 0;
    _dmc->enabled = (newStatus & 16) != 0;
}

void Apu::WriteApuPulse0(u8 val, NesAudioPulseCtrl* audioCtrl, ApuPulseState* state)
{
    int dutyCycle = (val >> 6) & 0x03;
    bool haltFlag = (val & 0x20) != 0;
    bool constantVolumeFlag = (val & 0x10) != 0;
    int volume = val & 0x0F;

    state->halt = haltFlag;
    state->constantVolume = constantVolumeFlag;
    audioCtrl->dutyCycle = dutyCycle;
    audioCtrl->volume = volume;
}

void Apu::WriteApuPulse1(u8 val, ApuPulseState* state)
{
    // Sweep unit

    // Not implemented
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

void Apu::WriteApuPulse3(u8 val, NesAudioPulseCtrl* audioCtrl, ApuPulseState* state)
{
    // Length counter not implemented

    state->wavelength &= 0x00FF;
    state->wavelength |= (val & 0x07) << 8;

    if (state->wavelength < 8)
        audioCtrl->frequency = 0;
    else
        audioCtrl->frequency = WavelengthToFrequency(false, state->wavelength);
}

void Apu::WriteApuTriangle0(u8 val)
{
    // Linear counter - not implemented
}

void Apu::WriteApuTriangle2(u8 val)
{
    _triangeWavelength &= 0xFF00;
    _triangeWavelength |= val;

    if (_triangeWavelength < 2)
        _triangle->frequency = 0;
    else
        _triangle->frequency = WavelengthToFrequency(true, _triangeWavelength);
}

void Apu::WriteApuTriangle3(u8 val)
{
    // Length counter not implemented

    _triangeWavelength &= 0x00FF;
    _triangeWavelength |= (val & 0x07) << 8;

    if (_triangeWavelength < 2)
        _triangle->frequency = 0;
    else
        _triangle->frequency = WavelengthToFrequency(true, _triangeWavelength);
}

void Apu::WriteApuNoise0(u8 val)
{
    // Not implemented
}

void Apu::WriteApuNoise2(u8 val)
{
    // Not implemented
}

void Apu::WriteApuNoise3(u8 val)
{
    // Not implemented
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
    // Not implemented
}

int Apu::WavelengthToFrequency(bool isTriangle, int wavelength)
{
    double steps = isTriangle ? TRIANGLE_WAVEFORM_STEPS : PULSE_WAVEFORM_STEPS;
    double cpuFreq = _isPal ? CPU_FREQ_PAL : CPU_FREQ_NTSC;
    return (int)(cpuFreq / (steps * (wavelength + 1)));
}