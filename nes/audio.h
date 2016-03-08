#pragma once

#define NESAUDIO_PULSE_DUTYCYCLE_12_5 0 // Pulse channel 12.5% duty cycle
#define NESAUDIO_PULSE_DUTYCYCLE_25 1 // Pulse channel 25% duty cycle
#define NESAUDIO_PULSE_DUTYCYCLE_50 2 // Pulse channel 50% duty cycle
#define NESAUDIO_PULSE_DUTYCYCLE_25N 3 // Pulse channel 25% negated duty cycle

struct NesAudioPulseCtrl
{
    volatile int lengthCounter;
    volatile int dutyCycle;
    volatile int frequency;
    volatile u8 volume;
    volatile bool phaseReset;
};

struct NesAudioTriangeCtrl
{
    volatile int lengthCounter;
    volatile int linearCounter;
    volatile int frequency;
};

struct NesAudioNoiseCtrl
{
    volatile int lengthCounter;
    volatile int period;
    volatile u8 volume;
    volatile bool mode1;
};

struct NesAudioDmcCtrl
{
    volatile u8 directLoad;
    volatile u8 sampleBuffer;
};

class AudioEngine
{
public:
    AudioEngine(
        NesAudioPulseCtrl* pulse1,
        NesAudioPulseCtrl* pulse2,
        NesAudioTriangeCtrl* triangle,
        NesAudioNoiseCtrl* noise,
        NesAudioDmcCtrl* dmc);
    virtual ~AudioEngine();

    void StartAudio(
        int preferredSampleRate,
        int pulseMinFreq,
        int pulseMaxFreq,
        int triangleMinFreq,
        int triangleMaxFreq);
    void StopAudio();
    void PauseAudio();
    void UnpauseAudio();

private:
    void InitializeTables();
    void ReleaseTables();

    void GenerateTable(int minFreq, int maxFreq, double (fourierSeriesFunction)(int phase, int harmonic), u8* wavetable);
    void GenarateOtherPulseTables();

    void AudioError(const char* error);

    void ExecuteCallback(u8 *stream, int len);
    u32 SampleWaveform(int freq, int freqStep, int volume, int& phase, int phaseDivider, u8* wavetable);
    u32 SamplePulse(NesAudioPulseCtrl* pulseChannel, int& phase, int phaseDivider);
    u32 SampleTriangle(int phaseDivider);
    u32 SampleNoise();
    void StepNoiseRegister();

private:
    SDL_AudioDeviceID _deviceId;
    int _sampleRate;
    u8 _silenceValue;

    int _nyquistFreq;
    int _pulseMinFreq;
    int _pulseMaxFreq;
    int _pulseFreqStep;
    int _triangleMinFreq;
    int _triangleMaxFreq;
    int _triangleFreqStep;

    int _pulse1Phase;
    int _pulse2Phase;
    int _trianglePhase;
    int _noisePeriodCounter;
    u8 _noiseAmplitude;
    u16 _noiseShiftRegister;

    // Wavetables
    u8* _pulseWavetableMemory;
    u8* _pulseWavetables[4];
    u8* _triangleWavetable;

    // These structures are updated by the APU on the emulator thread.  The audio engine generates
    // audio in real time based on the values in these structures.
    NesAudioPulseCtrl* _pulse1;
    NesAudioPulseCtrl* _pulse2;
    NesAudioTriangeCtrl* _triangle;
    NesAudioNoiseCtrl* _noise;
    NesAudioDmcCtrl* _dmc;

    friend void AudioGenerateCallback(void *userdata, u8 *stream, int len);
};