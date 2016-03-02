#pragma once

#define NESAUDIO_PULSE_DUTYCYCLE_12_5 0 // Pulse channel 12.5% duty cycle
#define NESAUDIO_PULSE_DUTYCYCLE_25 1 // Pulse channel 25% duty cycle
#define NESAUDIO_PULSE_DUTYCYCLE_50 2 // Pulse channel 50% duty cycle
#define NESAUDIO_PULSE_DUTYCYCLE_25N 3 // Pulse channel 25% negated duty cycle

struct NesAudioPulseCtrl
{
    volatile bool enabled;
    volatile int dutyCycle;
    volatile int frequency;
    volatile int volume;
};

struct NesAudioTriangeCtrl
{
    volatile bool enabled;
    volatile int frequency;
};

struct NesAudioNoiseCtrl
{
    volatile bool enabled;
};

struct NesAudioDmcCtrl
{
    volatile bool enabled;
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

    u8* GenerateTable(int minFreq, int maxFreq, double (fourierSeriesFunction)(int phase, int harmonic));

    void AudioError(const char* error);

    void ExecuteCallback(u8 *stream, int len);
    u8 SamplePulse1(int phaseDivider);
    u8 SamplePulse2(int phaseDivider);
    u8 SampleTriangle(int phaseDivider);

private:
    SDL_AudioDeviceID _deviceId;
    int _sampleRate;
    u8 _silenceValue;

    int _nyquistFreq;
    int _pulseMinFreq;
    int _pulseMaxFreq;
    int _triangleMinFreq;
    int _triangleMaxFreq;
    int _triangleFreqStep;

    int _pulse1Phase;
    int _pulse2Phase;
    int _trianglePhase;

    // Wavetables
    u8* _pulseWavetable50; // Pulse 50% duty cycle
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