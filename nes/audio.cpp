#include "stdafx.h"
#include "audio.h"

// Values tweaked for performance/audio quality
#define SAMPLE_BUFFER_SIZE 4096
#define WAVETABLE_FREQUENCY_STEPS 64
#define WAVETABLE_SAMPLES 256

static volatile unsigned int g_engineCount;

static void AudioGenerateCallback(void *userdata, u8 *stream, int len)
{
    ((AudioEngine*)userdata)->ExecuteCallback(stream, len);
}

// Functions for calculating Fourier series for each waveform
static double PulseFourierFunction50(int phase, int harmonic)
{
    // Square wave (50% duty cycle) Fourier series calculation.
    return (1.0 / harmonic) * sin(harmonic * M_PI * 2.0 * ((double)phase / WAVETABLE_SAMPLES));
}

static double TriangleFourierFunction(int phase, int harmonic)
{
    // Triange wave Fourier series calculation.
    return pow(-1.0, (harmonic - 1) / 2) * (1.0 / (harmonic * harmonic)) * sin(harmonic * M_PI * 2.0 * ((double)phase / WAVETABLE_SAMPLES));
}

AudioEngine::AudioEngine(
    NesAudioPulseCtrl* pulse1,
    NesAudioPulseCtrl* pulse2,
    NesAudioTriangeCtrl* triangle,
    NesAudioNoiseCtrl* noise,
    NesAudioDmcCtrl* dmc)
    : _deviceId(0)
    , _sampleRate(0)
    , _silenceValue(0)
    , _trianglePhase(0)
    , _pulse1Phase(0)
    , _pulse2Phase(0)
{
    _pulse1 = pulse1;
    _pulse2 = pulse2;
    _triangle = triangle;
    _noise = noise;
    _dmc = dmc;
}

AudioEngine::~AudioEngine()
{
    StopAudio();
}

void AudioEngine::StartAudio(
    int preferredSampleRate,
    int pulseMinFreq,
    int pulseMaxFreq,
    int triangleMinFreq,
    int triangleMaxFreq)
{
    unsigned int lastCount = InterlockedCompareExchange(&g_engineCount, 1, 0);
    if (lastCount != 0)
    {
        AudioError("Cannot start more than one audio engine at the same time");
        return;
    }

    SDL_AudioSpec desired = { 0 };
    SDL_AudioSpec obtained = { 0 };
    desired.format = AUDIO_U8;
    desired.channels = 1;
    desired.freq = preferredSampleRate;
    desired.samples = SAMPLE_BUFFER_SIZE;
    desired.callback = AudioGenerateCallback;
    desired.userdata = this;

    _deviceId = SDL_OpenAudioDevice(
        nullptr /* Choose reasonable default device */,
        false /* Capture disabled */,
        &desired,
        &obtained,
        SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
    if (_deviceId == 0)
    {
        AudioError(SDL_GetError());
        return;
    }

    _sampleRate = obtained.freq;
    _silenceValue = obtained.silence;

    _nyquistFreq = _sampleRate / 2;
    _pulseMinFreq = pulseMinFreq;
    _pulseMaxFreq = pulseMaxFreq;
    _triangleMinFreq = triangleMinFreq;
    _triangleMaxFreq = triangleMaxFreq;

    // We can't synthesize any frequency above the Nyquist frequency.
    // Clip the max frequencies at the Nyquist frequency.
    if (_pulseMaxFreq > _nyquistFreq)
        _pulseMaxFreq = _nyquistFreq;
    if (_triangleMaxFreq > _nyquistFreq)
        _triangleMaxFreq = _nyquistFreq;

    InitializeTables();
    UnpauseAudio();
}

void AudioEngine::StopAudio()
{
    if (_deviceId != 0)
    {
        SDL_CloseAudioDevice(_deviceId);
        ReleaseTables();

        _deviceId = 0;
    }

    g_engineCount = 0;
}

void AudioEngine::PauseAudio()
{
    SDL_PauseAudioDevice(_deviceId, 1);
}

void AudioEngine::UnpauseAudio()
{
    SDL_PauseAudioDevice(_deviceId, 0);
}

void AudioEngine::InitializeTables()
{
    _pulseWavetable50 = GenerateTable(_pulseMinFreq, _pulseMaxFreq, PulseFourierFunction50);
    _triangleWavetable = GenerateTable(_triangleMinFreq, _triangleMaxFreq, TriangleFourierFunction);
}

void AudioEngine::ReleaseTables()
{
    if (_pulseWavetable50 != nullptr)
    {
        delete[] _pulseWavetable50;
        _pulseWavetable50 = nullptr;
    }

    if (_triangleWavetable != nullptr)
    {
        delete[] _triangleWavetable;
        _triangleWavetable = nullptr;
    }
}

u8* AudioEngine::GenerateTable(int minFreq, int maxFreq, double (fourierSeriesFunction)(int phase, int harmonic))
{
    double* rawTable = new double[WAVETABLE_FREQUENCY_STEPS * WAVETABLE_SAMPLES];
    double minSample = 1.0;
    double maxSample = -1.0;

    int freq = minFreq;
    int freqStep = (int)((maxFreq - minFreq) / WAVETABLE_FREQUENCY_STEPS);
    for (int f = 0; f < WAVETABLE_FREQUENCY_STEPS; f++)
    {
        freq += freqStep;

        double* tableRow = rawTable + f * WAVETABLE_SAMPLES;
        int numHarmonics = _nyquistFreq / freq; // Only generate harmonics below the Nyquist frequency
        for (int phase = 0; phase < WAVETABLE_SAMPLES; phase++)
        {
            double sum = 0.0;

            // We only need odd harmonics for all waveforms we generate so we increment by 2
            for (int harmonic = 1; harmonic <= numHarmonics; harmonic += 2)
            {
                // Calculate the factor we need to multiply this harmonic by to compensate for
                // the Gibbs phenomenon.
                double gibbsFactor = cos((harmonic - 1) * M_PI / (2.0 * numHarmonics));
                gibbsFactor *= gibbsFactor;

                // Call the passed in function to get value of the Fourier series for the waveform
                // at the current phase/harmonic.
                sum += gibbsFactor * fourierSeriesFunction(phase, harmonic);
            }

            tableRow[phase] = sum;
            if (sum > maxSample)
                maxSample = sum;
            if (sum < minSample)
                minSample = sum;
        }
    }

    // Now normalize the calculated values so the wavetable only contains 0-255 value samples
    u8* normalizedTable = new u8[WAVETABLE_FREQUENCY_STEPS * WAVETABLE_SAMPLES];
    double range = maxSample - minSample;
    for (int row = 0; row < WAVETABLE_FREQUENCY_STEPS; row++)
    {
        double* rawRow = rawTable + row * WAVETABLE_SAMPLES;
        u8* normalizedRow = normalizedTable + row * WAVETABLE_SAMPLES;
        for (int column = 0; column < WAVETABLE_SAMPLES; column++)
        {
            normalizedRow[column] = (byte)((rawRow[column] - minSample) * 255.0 / range);
        }
    }

    delete[] rawTable;
    return normalizedTable;
}

void AudioEngine::AudioError(const char* error)
{
    // TODO: Do something different in the emulator?
    printf_s("Audio Engine Error: %s", error);
}

void AudioEngine::ExecuteCallback(u8 *stream, int len)
{
    int phaseDivider = _sampleRate / WAVETABLE_SAMPLES;
    for (int i = 0; i < len; i++)
    {
        int value = 0;
        value += SamplePulse1(phaseDivider);
        value += SamplePulse2(phaseDivider);
        value += SampleTriangle(phaseDivider);

        stream[i] = (u8)(value / 3);
    }
}

u8 AudioEngine::SamplePulse1(int phaseDivider)
{
    int freq = _pulse1->frequency;
    if (_pulse1->enabled && freq > 0)
    {
        int rowIndex = freq / (int)((_pulseMaxFreq - _pulseMinFreq) / WAVETABLE_FREQUENCY_STEPS);
        int phaseIndex = _pulse1Phase / phaseDivider;
        u8 sample = (_pulseWavetable50 + (rowIndex * WAVETABLE_FREQUENCY_STEPS))[phaseIndex];

        _pulse1Phase += freq;
        if (_pulse1Phase >= _sampleRate)
            _pulse1Phase -= _sampleRate;

        return sample / 8; // volume is NYI
    }
    else
    {
        return _silenceValue;
    }
}

u8 AudioEngine::SamplePulse2(int phaseDivider)
{
    int freq = _pulse2->frequency;
    if (_pulse2->enabled && freq > 0)
    {
        int rowIndex = freq / (int)((_pulseMaxFreq - _pulseMinFreq) / WAVETABLE_FREQUENCY_STEPS);
        int phaseIndex = _pulse2Phase / phaseDivider;
        u8 sample = (_pulseWavetable50 + (rowIndex * WAVETABLE_FREQUENCY_STEPS))[phaseIndex];

        _pulse2Phase += freq;
        if (_pulse2Phase >= _sampleRate)
            _pulse2Phase -= _sampleRate;

        return sample / 8; // volume is NYI
    }
    else
    {
        return _silenceValue;
    }
}

u8 AudioEngine::SampleTriangle(int phaseDivider)
{
    int freq = _triangle->frequency;
    if (_triangle->enabled && freq > 0)
    {
        int rowIndex = freq / (int)((_triangleMaxFreq - _triangleMinFreq) / WAVETABLE_FREQUENCY_STEPS);
        int phaseIndex = _trianglePhase / phaseDivider;
        u8 sample = (_triangleWavetable + (rowIndex * WAVETABLE_FREQUENCY_STEPS))[phaseIndex];

        _trianglePhase += freq;
        if (_trianglePhase >= _sampleRate)
            _trianglePhase -= _sampleRate;

        return sample / 4; // volume is NYI
    }
    else
    {
        return _silenceValue;
    }
}
