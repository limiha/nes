#pragma once

#include "eventqueue.h"

struct IAudioProvider;

enum WavetableIndex
{
    NESAUDIO_PULSE_DUTYCYCLE_12_5 = 0, // Pulse channel 12.5% duty cycle
    NESAUDIO_PULSE_DUTYCYCLE_25 = 1, // Pulse channel 25% duty cycle
    NESAUDIO_PULSE_DUTYCYCLE_50 = 2, // Pulse channel 50% duty cycle
    NESAUDIO_PULSE_DUTYCYCLE_25N = 3, // Pulse channel 25% negated duty cycle
    NESAUDIO_TRIANGE = 4,

    NESAUDIO_NUM_WAVETABLES
};

enum AudioChannelSetting
{
    NESAUDIO_CHANNEL_SETTING_NONE,

    NESAUDIO_FRAME_RESET,
    NESAUDIO_PULSE1_DUTYCYCLE,
    NESAUDIO_PULSE1_FREQUENCY,
    NESAUDIO_PULSE1_VOLUME,
    NESAUDIO_PULSE1_PHASE_RESET,
    NESAUDIO_PULSE2_DUTYCYCLE,
    NESAUDIO_PULSE2_FREQUENCY,
    NESAUDIO_PULSE2_VOLUME,
    NESAUDIO_PULSE2_PHASE_RESET,
    NESAUDIO_TRIANGLE_FREQUENCY,
    NESAUDIO_NOISE_PERIOD,
    NESAUDIO_NOISE_MODE,
    NESAUDIO_NOISE_VOLUME,
    NESAUDIO_DMC_VALUE,

    NESAUDIO_CHANNEL_NUM_SETTINGS
};

struct AudioEvent
{
    u16 cpuCycleCount;
    u16 audioSetting;
    u32 newValue;
};

struct WavetableChannel
{
    u32 frequency;
    u32 freqStep;
    u32 phase;
    u32 volume;
    u32 wavetable;
    u8* wavetableRow;
    u8 lastSample;

    WavetableChannel()
        : frequency(0)
        , phase(0)
        , volume(0)
        , wavetable(0)
        , wavetableRow(0)
        , lastSample(0)
    {
    }
};

class AudioEngine
{
public:
    AudioEngine(std::shared_ptr<IAudioProvider> audioProvider);
    virtual ~AudioEngine();

    void StartAudio(
        int preferredSampleRate,
        int cpuFreq,
        int pulseMinFreq,
        int pulseMaxFreq,
        int triangleMinFreq,
        int triangleMaxFreq);
    void StopAudio();
    void PauseAudio();
    void UnpauseAudio();

    void QueueAudioEvent(int cycleCount, int setting, u32 newValue);

private:
    void InitializeTables();
    void InitializeChannels();
    void ReleaseTables();

    void GenerateTable(int minFreq, int maxFreq, double (fourierSeriesFunction)(int phase, int harmonic), u8* wavetable);
    void GenarateOtherPulseTables();

    void ExecuteCallback(u8* stream, int len);
    void GenerateSamples(u8* stream, int count);
    void ProcessAudioEvents();
    void ProcessAudioEvent(const AudioEvent& event);
    void UpdateWavetableRow(WavetableChannel& channel);

    i32 SampleWavetableChannel(WavetableChannel& channel);
    i32 SampleNoise();

    void StepNoiseRegister();
    int CycleCountToSampleCount(double cycleCount);
    static void BoundFrequency(u32& frequency, u32 minFreq, u32 maxFreq);

private:
    // Audio device info
    std::shared_ptr<IAudioProvider> _audioProvider;
    bool _audioStarted;
    int _sampleRate;
    u8 _silenceValue;

    // Constants based on CPU frequency and sample rate
    u32 _cpuFreq;
    u32 _nyquistFreq;
    u32 _pulseMinFreq;
    u32 _pulseMaxFreq;
    u32 _pulseFreqStep;
    u32 _triangleMinFreq;
    u32 _triangleMaxFreq;
    u32 _triangleFreqStep;
    u32 _phaseDivider;
    double _cyclesPerSample;

    // Wavetables
    u8* _wavetableMemory;
    u8* _wavetables[NESAUDIO_NUM_WAVETABLES];

    // Audio engine state information
    EventQueue<AudioEvent> _eventQueue;
    std::atomic<u32> _pendingFrameResetCount;
    AudioEvent _nextEvent;
    int _samplesRemaining;
    bool _eventPending;
    double _cycleCounter;

    // Channels (except for initialization, these should only be read/modified on the callback thread)
    WavetableChannel _pulseChannel1;
    WavetableChannel _pulseChannel2;
    WavetableChannel _triangleChannel;
    i32 _dmcAmplitude;
    u16 _noiseShiftRegister;
    u32 _noiseVolume;
    bool _noiseMode1;
    bool _noiseOn;
    double _noisePeriodSamples;
    double _noiseCounter;

    friend void AudioGenerateCallback(void *userdata, u8 *stream, int len);
};