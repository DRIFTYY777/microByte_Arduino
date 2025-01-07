#ifndef SOUND_H
#define SOUND_H

#include <cstdint>

#define AUDIO_SAMPLE_8KHZ 8000
#define AUDIO_SAMPLE_16KHZ 16000
#define AUDIO_SAMPLE_32KHZ 32000
#define AUDIO_SAMPLE_44kHZ 44100

class SOUND
{
private:
    float volume_level;

public:
    bool audio_init(uint32_t sample_rate);
    void audio_submit(short *stereoAudioBuffer, uint32_t frameCount);
    void audio_terminate();
    uint8_t audio_volume_get();
    void audio_volume_set(float level);
};

extern SOUND sound;

#endif // SOUND_H