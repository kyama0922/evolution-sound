#ifndef __EVOLUTION_WAV_LOADER_H__
#define __EVOLUTION_WAV_LOADER_H__

#include "../evolution_sound_interface.h"

#if defined(WIN32) | defined(WIN64)
#include <Windows.h>
namespace EVOLUTION{
    namespace SOUND{
        class SoundFile_WavLoader :public ISoundFileLoader{
        private:
            InstanceCounter m_instance_counter;
            u8* mp_sound_buffer;
            u32 m_sound_buffer_size;
            WAVEFORMATEX m_wav_format;
        public:
            u32 AddRef();
            RESULT QueryInterface(EVOLUTION_IID riid, void **ppvObject);
            u32 Release();

            SoundFile_WavLoader(const char* file_name);
            SoundFile_WavLoader(const wchar_t* file_name);

            const u8* GetSoundBuffer()const;
            u32 GetSoundBufferSize()const;
            const WAVEFORMATEX& GetSoundFormat()const;
        };
    }
    namespace EVOLUTION_GUID{
        // {970DF0AA-9942-481d-8FF2-0B81C3B8B284}
        static const EVOLUTION_IID IID_SoundFile_WavLoader =
        { 0x970df0aa, 0x9942, 0x481d, { 0x8f, 0xf2, 0xb, 0x81, 0xc3, 0xb8, 0xb2, 0x84 } };

    }
}
#endif//defined(WIN32) | defined(WIN64)

#endif //!__EVOLUTION_WAV_LOADER_H__