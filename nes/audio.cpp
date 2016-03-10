#include "stdafx.h"
#include "audio.h"

// Values tweaked for performance/audio quality
#define SAMPLE_BUFFER_SIZE 1024
#define WAIT_NUM_SAMPLES 32 // number of samples to play with current setting when no events are available
#define WAVETABLE_FREQUENCY_STEPS 64
#define WAVETABLE_SAMPLES 256
#define WAVETABLE_SIZE WAVETABLE_FREQUENCY_STEPS * WAVETABLE_SAMPLES

#define MAX_FRAME_CYCLE_COUNT 38000 // Larger than max frame clock cycle count, but can't exceed max unsigned 16-bit integer

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

AudioEngine::AudioEngine()
    : _deviceId(0)
    , _sampleRate(0)
    , _silenceValue(0)
    , _eventQueue(MAX_FRAME_CYCLE_COUNT)
    , _pendingFrameResetCount(0)
    , _samplesRemaining(0)
    , _eventPending(true)
    , _cycleCounter(0)
    , _wavetableMemory(nullptr)
{
    memset(&_nextEvent, 0, sizeof(AudioEvent));
    memset(&_pulseChannel1, 0, sizeof(WavetableChannel));
    memset(&_pulseChannel2, 0, sizeof(WavetableChannel));
    memset(&_triangleChannel, 0, sizeof(WavetableChannel));
    memset(&_wavetables, 0, sizeof(_wavetables));
}

AudioEngine::~AudioEngine()
{
    StopAudio();
}

void AudioEngine::StartAudio(
    int preferredSampleRate,
    int cpuFreq,
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

    _cpuFreq = cpuFreq;
    _nyquistFreq = _sampleRate / 2;
    _pulseMinFreq = pulseMinFreq;
    _pulseMaxFreq = pulseMaxFreq;
    _pulseFreqStep = (int)((_pulseMaxFreq - _pulseMinFreq) / WAVETABLE_FREQUENCY_STEPS);
    _triangleMinFreq = triangleMinFreq;
    _triangleMaxFreq = triangleMaxFreq;
    _triangleFreqStep = (int)((_triangleMaxFreq - _triangleMinFreq) / WAVETABLE_FREQUENCY_STEPS);
    _phaseDivider = _sampleRate / WAVETABLE_SAMPLES;
    _cyclesPerSample = (double)_cpuFreq / _sampleRate;

    // We can't synthesize any frequency above the Nyquist frequency.
    // Clip the max frequencies at the Nyquist frequency.
    if (_pulseMaxFreq > _nyquistFreq)
        _pulseMaxFreq = _nyquistFreq;
    if (_triangleMaxFreq > _nyquistFreq)
        _triangleMaxFreq = _nyquistFreq;

    InitializeTables();
    InitializeChannels();

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

void AudioEngine::QueueAudioEvent(int cycleCount, int setting, u32 newValue)
{
    AudioEvent event;
    event.cpuCycleCount = cycleCount;
    event.audioSetting = setting;
    event.newValue = newValue;

    // TODO: Currently we drop events if the queue is full.
    // We need to figure out a better way to handle this.
    bool queued = _eventQueue.EnqueueEvent(event);
    if (!queued)
        __debugbreak();

    if (event.audioSetting == NESAUDIO_FRAME_RESET)
        InterlockedIncrement(&_pendingFrameResetCount);
}

#define INIT_WAVETABLE_PTR(WT) _wavetables[WT] = _wavetableMemory + WAVETABLE_SIZE * (WT)
#define UNINIT_WAVETABLE_PTR(WT) _wavetables[WT] = nullptr

void AudioEngine::InitializeTables()
{
    _wavetableMemory = new u8[WAVETABLE_SIZE * NESAUDIO_NUM_WAVETABLES];
    INIT_WAVETABLE_PTR(NESAUDIO_PULSE_DUTYCYCLE_12_5);
    INIT_WAVETABLE_PTR(NESAUDIO_PULSE_DUTYCYCLE_25);
    INIT_WAVETABLE_PTR(NESAUDIO_PULSE_DUTYCYCLE_25N);
    INIT_WAVETABLE_PTR(NESAUDIO_PULSE_DUTYCYCLE_50);
    INIT_WAVETABLE_PTR(NESAUDIO_TRIANGE);

    GenerateTable(_pulseMinFreq, _pulseMaxFreq, PulseFourierFunction50, _wavetables[NESAUDIO_PULSE_DUTYCYCLE_50]);
    GenerateTable(_triangleMinFreq, _triangleMaxFreq, TriangleFourierFunction, _wavetables[NESAUDIO_TRIANGE]);
    GenarateOtherPulseTables();
}

void AudioEngine::ReleaseTables()
{
    UNINIT_WAVETABLE_PTR(NESAUDIO_PULSE_DUTYCYCLE_12_5);
    UNINIT_WAVETABLE_PTR(NESAUDIO_PULSE_DUTYCYCLE_25);
    UNINIT_WAVETABLE_PTR(NESAUDIO_PULSE_DUTYCYCLE_25N);
    UNINIT_WAVETABLE_PTR(NESAUDIO_PULSE_DUTYCYCLE_50);
    UNINIT_WAVETABLE_PTR(NESAUDIO_TRIANGE);

    if (_wavetableMemory != nullptr)
    {
        delete[] _wavetableMemory;
        _wavetableMemory = nullptr;
    }
}

#undef INIT_WAVETABLE
#undef UNINIT_WAVETABLE_PTR

void AudioEngine::InitializeChannels()
{
    _pulseChannel1.wavetable = NESAUDIO_PULSE_DUTYCYCLE_50;
    _pulseChannel1.freqStep = _pulseFreqStep;
    _pulseChannel2.wavetable = NESAUDIO_PULSE_DUTYCYCLE_50;
    _pulseChannel2.freqStep = _pulseFreqStep;

    _triangleChannel.wavetable = NESAUDIO_TRIANGE;
    _triangleChannel.freqStep = _triangleFreqStep;
    _triangleChannel.volume = 0x0F;

    _dmcAmplitude = 0;

    _noiseShiftRegister = 1;
    _noiseVolume = 0;
    _noiseMode1 = false;
    _noisePeriodSamples = 0.0;
    _noiseCounter = 0.0;
}

void AudioEngine::GenerateTable(int minFreq, int maxFreq, double (fourierSeriesFunction)(int phase, int harmonic), u8* wavetable)
{
    double* rawTable = new double[WAVETABLE_SIZE];
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
    double range = maxSample - minSample;
    for (int row = 0; row < WAVETABLE_FREQUENCY_STEPS; row++)
    {
        double* rawRow = rawTable + row * WAVETABLE_SAMPLES;
        u8* normalizedRow = wavetable + row * WAVETABLE_SAMPLES;
        for (int column = 0; column < WAVETABLE_SAMPLES; column++)
        {
            normalizedRow[column] = (byte)((rawRow[column] - minSample) * 255.0 / range);
        }
    }

    delete[] rawTable;
}

void AudioEngine::GenarateOtherPulseTables()
{
    int halfWave = WAVETABLE_SAMPLES / 2;
    int quarterWave = halfWave / 2;
    int eighthWave = quarterWave / 2;
    int sixteenthWave = eighthWave / 2;

    for (int row = 0; row < WAVETABLE_FREQUENCY_STEPS; row++)
    {
        u8* tableRow12_5 = _wavetables[NESAUDIO_PULSE_DUTYCYCLE_12_5] + row * WAVETABLE_SAMPLES;
        u8* tableRow25 = _wavetables[NESAUDIO_PULSE_DUTYCYCLE_25] + row * WAVETABLE_SAMPLES;
        u8* tableRow50 = _wavetables[NESAUDIO_PULSE_DUTYCYCLE_50] + row * WAVETABLE_SAMPLES;
        u8* tableRow25N = _wavetables[NESAUDIO_PULSE_DUTYCYCLE_25N] + row * WAVETABLE_SAMPLES;
        int square12phase = 0;
        int square25phase = 0;
        int square50phase = 0;

        // Create 25% and 25% negated duty cycle pulse tables

        // Copy ascending portion of square wave
        while (square25phase < eighthWave)
        {
            tableRow25[square25phase] = tableRow50[square50phase];
            tableRow25N[square25phase++] = 255 - tableRow50[square50phase++];
        }

        // Copy next 1/4+1/8 of the the square wave starting at the 1/8 point of the 25% wave
        // (we'll end up at the half-way point)
        square50phase += quarterWave;
        while (square25phase < halfWave)
        {
            tableRow25[square25phase] = tableRow50[square50phase];
            tableRow25N[square25phase++] = 255 - tableRow50[square50phase++];
        }

        // Duplicate the current sample for 1/4 of the 25% wave
        while (square25phase < halfWave + quarterWave)
        {
            tableRow25[square25phase] = tableRow50[square50phase];
            tableRow25N[square25phase++] = 255 - tableRow50[square50phase];
        }

        // Copy the final 1/4 of the wave
        while (square25phase < WAVETABLE_SAMPLES)
        {
            tableRow25[square25phase] = tableRow50[square50phase];
            tableRow25N[square25phase++] = 255 - tableRow50[square50phase++];
        }

        // Create the 12.5% duty cycle pulse table

        // Copy ascending portion of square wave
        square50phase = 0;
        while (square12phase < sixteenthWave)
            tableRow12_5[square12phase++] = tableRow50[square50phase++];

        // Copy next 1/4+1/16 of the the square wave starting at the 1/16 point of the 25% wave
        square50phase += quarterWave + eighthWave;
        while (square12phase < halfWave)
            tableRow12_5[square12phase++] = tableRow50[square50phase++];

        // Duplicate the current sample for 3/8 of the 25% wave
        while (square12phase < halfWave + eighthWave * 3)
            tableRow12_5[square12phase++] = tableRow50[square50phase];

        // Copy the rest of the wave
        while (square12phase < WAVETABLE_SAMPLES)
            tableRow12_5[square12phase++] = tableRow50[square50phase++];
    }
}

void AudioEngine::AudioError(const char* error)
{
    // TODO: Do something different in the emulator?
    printf_s("Audio Engine Error: %s", error);
}

void AudioEngine::ExecuteCallback(u8 *stream, int len)
{
    for (;;)
    {
        // Generate samples
        if (_samplesRemaining > 0)
        {
            int sampleCount = _samplesRemaining;
            if (sampleCount >= len)
            {
                sampleCount = len;
                GenerateSamples(stream, sampleCount);

                // Buffer has been filled completely.
                // Update samples remaining and return.
                _samplesRemaining -= sampleCount;
                return;
            }
            else
            {
                GenerateSamples(stream, sampleCount);

                stream += sampleCount;
                len -= sampleCount;
                _samplesRemaining = 0;
            }
        }

        // Process audio events from the emulator
        ProcessAudioEvents();
        if (!_eventPending)
        {
            // Event queue is empty.
            //
            // Do a few more samples with the current channel settings or finish the buffer if it
            // is almost full.
            _samplesRemaining = WAIT_NUM_SAMPLES;
            if (_samplesRemaining > len)
                _samplesRemaining = len;
        }
    }
}

void AudioEngine::GenerateSamples(u8* stream, int count)
{
    for (int i = 0; i < count; i++)
    {
        i32 pulseSample;
        i32 triangleSample;
        i32 noiseSample;

        pulseSample = SampleWavetableChannel(_pulseChannel1) + SampleWavetableChannel(_pulseChannel2);
        triangleSample = SampleWavetableChannel(_triangleChannel);
        noiseSample = SampleNoise();

        double output = 2.95e-5 * pulseSample +
            3.34e-5 * triangleSample +
            0.00247 * noiseSample +
            0.00335 * _dmcAmplitude;

        *stream++ = (u8)(output * _silenceValue + _silenceValue);
    }

    // Update the cycle counter
    _cycleCounter += _cyclesPerSample * count;
}

void AudioEngine::ProcessAudioEvents()
{
    if (_eventPending)
    {
        // There's already an event pending.
        ProcessAudioEvent(_nextEvent);
        _eventPending = false;
    }

    // Drain event queue of any past events and grab the next future event if there is one
DrainNextEvent:
    _eventPending = _eventQueue.DequeueEvent(_nextEvent);
    if (_eventPending)
    {
        if (_nextEvent.cpuCycleCount <= _cycleCounter || _pendingFrameResetCount > 1)
        {
            ProcessAudioEvent(_nextEvent);
            goto DrainNextEvent;
        }
        else
        {
            // Determine how long we need to wait (in samples) before processing the next event.
            _samplesRemaining = CycleCountToSampleCount(_nextEvent.cpuCycleCount - _cycleCounter);
            if (_samplesRemaining == 0)
            {
                ProcessAudioEvent(_nextEvent);
                goto DrainNextEvent;
            }
        }
    }
}

void AudioEngine::ProcessAudioEvent(const AudioEvent& event)
{
    u32 setting = event.newValue;
    switch (event.audioSetting)
    {
    case NESAUDIO_FRAME_RESET:
        // The APU frame counter has reset.  Reset our cycle counter.
        InterlockedDecrement(&_pendingFrameResetCount);
        _cycleCounter = 0;
        break;
    case NESAUDIO_PULSE1_DUTYCYCLE:
        _pulseChannel1.wavetable = setting & 3;
        UpdateWavetableRow(_pulseChannel1);
        break;
    case NESAUDIO_PULSE1_FREQUENCY:
        BoundFrequency(setting, _pulseMinFreq, _pulseMaxFreq);
        _pulseChannel1.frequency = setting;
        UpdateWavetableRow(_pulseChannel1);
        break;
    case NESAUDIO_PULSE1_VOLUME:
        _pulseChannel1.volume = setting;
        break;
    case NESAUDIO_PULSE1_PHASE_RESET:
        _pulseChannel1.phase = 0;
        break;
    case NESAUDIO_PULSE2_DUTYCYCLE:
        _pulseChannel2.wavetable = setting & 3;
        UpdateWavetableRow(_pulseChannel2);
        break;
    case NESAUDIO_PULSE2_FREQUENCY:
        BoundFrequency(setting, _pulseMinFreq, _pulseMaxFreq);
        _pulseChannel2.frequency = setting;
        UpdateWavetableRow(_pulseChannel2);
        break;
    case NESAUDIO_PULSE2_VOLUME:
        _pulseChannel2.volume = setting;
        break;
    case NESAUDIO_PULSE2_PHASE_RESET:
        _pulseChannel2.phase = 0;
        break;
    case NESAUDIO_TRIANGLE_FREQUENCY:
        UpdateTriangleFrequency(setting);
        break;
    case NESAUDIO_NOISE_PERIOD:
        if (setting == 0)
            _noisePeriodSamples = 0.0;
        else
            _noisePeriodSamples = (double)_sampleRate / ((double)_cpuFreq / 2.0 / setting);
        break;
    case NESAUDIO_NOISE_MODE:
        _noiseMode1 = setting != 0;
        break;
    case NESAUDIO_NOISE_VOLUME:
        _noiseVolume = setting;
        break;
    case NESAUDIO_DMC_VALUE:
        _dmcAmplitude = setting;
        break;
    }
}

void AudioEngine::UpdateTriangleFrequency(u32 newFreq)
{
    u32 oldFreq = _triangleChannel.frequency;
    BoundFrequency(newFreq, _triangleMinFreq, _triangleMaxFreq);
    _triangleChannel.newFreq = newFreq;

    // Ramp volume up/down for large changes in triangle frequency to avoid annoying
    // audio pops.
    if (oldFreq == 0 && newFreq != 0)
    {
        _triangleChannel.rampUp = true;
        _triangleChannel.rampDown = false;

        _triangleChannel.frequency = newFreq;
        UpdateWavetableRow(_triangleChannel);
    }
    else if (oldFreq != 0 && newFreq == 0)
    {
        _triangleChannel.rampUp = false;
        _triangleChannel.rampDown = true;
    }
    else
    {
        u32 diff = 0;
        if (oldFreq > newFreq)
            diff = oldFreq - newFreq;
        else
            diff = newFreq - oldFreq;

        _triangleChannel.rampUp = false;
        _triangleChannel.rampDown = true;
    }
}

void AudioEngine::UpdateWavetableRow(WavetableChannel& channel)
{
    int rowIndex = channel.frequency / channel.freqStep;
    channel.wavetableRow = _wavetables[channel.wavetable] + rowIndex * WAVETABLE_FREQUENCY_STEPS;
}

i32 AudioEngine::SampleWavetableChannel(WavetableChannel& channel)
{
    if (channel.frequency != 0)
    {
        int phase = channel.phase;
        int phaseIndex = phase / _phaseDivider;
        u8 sample = channel.wavetableRow[phaseIndex];

        phase += channel.frequency;
        if (phase >= _sampleRate)
            phase -= _sampleRate;

        channel.phase = phase;

        i32 result = (i32)sample * channel.volume;

        if (channel.rampDown)
        {
            if (channel.volume == 0)
            {
                channel.rampUp = true;
                channel.rampDown = false;
            }
            else if (--channel.volume == 0)
            {
                channel.rampDown = false;
                channel.frequency = channel.newFreq;
                if (channel.newFreq != 0)
                {
                    UpdateWavetableRow(channel);
                    channel.rampUp = true;
                }
            }
        }
        else if (channel.rampUp)
        {
            if (channel.volume == 0x0F)
                channel.rampUp = false;
            else if (++channel.volume == 0x0F)
                channel.rampUp = false;
        }

        return result;
    }
    else
    {
        return 0;
    }
}

i32 AudioEngine::SampleNoise()
{
    u32 sample = _silenceValue;
    if (_noisePeriodSamples != 0.0)
    {
        if (_noiseCounter <= 0.0)
        {
            StepNoiseRegister();
            _noiseCounter += _noisePeriodSamples;
        }
        else
        {
            _noiseCounter -= 1.0;
        }

        return _noiseOn ? _noiseVolume : 0;
    }

    return 0;
}

void AudioEngine::StepNoiseRegister()
{
    u16 feedback = _noiseShiftRegister & 1;

    if (_noiseMode1)
        feedback ^= (_noiseShiftRegister & 0x0040) >> 6;
    else
        feedback ^= (_noiseShiftRegister & 0x0002) >> 1;

    _noiseShiftRegister >>= 1;
    _noiseShiftRegister |= feedback << 15;
    _noiseOn = (_noiseShiftRegister & 1) != 0;
}

int AudioEngine::CycleCountToSampleCount(double cycleCount)
{
    return (int)(cycleCount / _cyclesPerSample);
}

void AudioEngine::BoundFrequency(u32& frequency, u32 minFreq, u32 maxFreq)
{
    if (frequency < minFreq)
        frequency = 0;
    if (frequency > maxFreq)
        frequency = 0;
}