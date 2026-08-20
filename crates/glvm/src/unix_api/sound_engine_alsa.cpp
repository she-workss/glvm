#include "glvm/sound_engine_alsa.hpp"

namespace glvm::core::Sound {
void CSoundEngineAlsa::OpenDevice(const char* device) {
    (snd_pcm_open(&pPcm, device, SND_PCM_STREAM_PLAYBACK, 0));
}

void CSoundEngineAlsa::CloseDevice() {
    snd_pcm_drain(pPcm);
    snd_pcm_close(pPcm);
}

void CSoundEngineAlsa::SoundStream() {
    for (unsigned int i = 0; i < tSound_Contaier.size(); ++i) {
        PlaybackSoundSample(*tSound_Contaier[i]);
        tSound_Contaier.erase(tSound_Contaier.begin() + i);
    }
}

void CSoundEngineAlsa::PlaybackSoundSample(CSoundSample& _sound_sample) {
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
    snd_pcm_access_t access = SND_PCM_ACCESS_RW_INTERLEAVED;
    unsigned int uiChannels = 2, uiRate;
    // 0.5 s
    unsigned int uiLatency = 500000;
    unsigned int uiFrame_Size = uiChannels * 2;

    uiRate = _sound_sample.uiRate_;
    (snd_pcm_set_params(pPcm, format, access, uiChannels, uiRate, 1, uiLatency));

#define FRAMES 32
    char *buf, *data;
    int frames, rest;
    FILE* iFile_Descritor;

    iFile_Descritor = fopen(_sound_sample.kPath_to_File_, "r");

    buf = (char*)malloc(FRAMES * uiFrame_Size);
    for (int i = 0; i < 300; ++i) {
        frames = fread(buf, uiFrame_Size, FRAMES, iFile_Descritor);
        if (frames <= 0) {
            break;
        }
        rest = frames;

        int16_t* samples = reinterpret_cast<int16_t*>(buf);

        int sampleCount = frames * uiChannels;

        for (int i = 0; i < sampleCount; ++i) {
            int32_t s = static_cast<int32_t>(samples[i] * _sound_sample.volume);

            s = std::clamp(s, -32768, 32767);

            samples[i] = static_cast<int16_t>(s);
        }

        data = buf;
        while (rest > 0) {
            frames = snd_pcm_writei(pPcm, data, rest);
            rest -= frames;
            data += frames * uiFrame_Size;
        }
    }
    free(buf);
}

void CSoundEngineAlsa::SetMasterVolume(long _lVolume) {
    long lMin, lMax;
    snd_mixer_t* pHandle;
    snd_mixer_selem_id_t* pSid;
    const char* pCard = "default";
    const char* pSelem_Name = "Master";

    snd_mixer_open(&pHandle, 0);
    snd_mixer_attach(pHandle, pCard);
    snd_mixer_selem_register(pHandle, NULL, NULL);
    snd_mixer_load(pHandle);

    snd_mixer_selem_id_alloca(&pSid);
    snd_mixer_selem_id_set_index(pSid, 0);
    snd_mixer_selem_id_set_name(pSid, pSelem_Name);
    snd_mixer_elem_t* pElem = snd_mixer_find_selem(pHandle, pSid);

    snd_mixer_selem_get_playback_volume_range(pElem, &lMin, &lMax);
    snd_mixer_selem_set_playback_volume_all(
        pElem,
        lMin + (_lVolume * (lMax - lMin)) / 100
    );

    snd_mixer_close(pHandle);
}

std::vector<CSoundSample*>& CSoundEngineAlsa::GetSoundContainer() {
    return tSound_Contaier;
}

void CSoundEngineAlsa::CreateSoundSample(
    const char* filePath,
    uint32_t duration,
    uint32_t rate,
    float volume
) {
    core::Sound::CSoundSample* pSound_Sample = new core::Sound::CSoundSample();
    pSound_Sample->kPath_to_File_ = filePath;
    pSound_Sample->uiDuration_ = duration;
    pSound_Sample->uiRate_ = rate;
    pSound_Sample->volume = volume;
    tSound_Contaier.push_back(pSound_Sample);
}

CSoundEngineAlsa::~CSoundEngineAlsa() {
    for (uint32_t i = 0; i < tSound_Contaier.size(); ++i) {
        delete tSound_Contaier[i];
        tSound_Contaier[i] = nullptr;
    }
}
} // namespace glvm::core::Sound
