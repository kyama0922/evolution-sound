#include "evolution_soundimplement.h"


using namespace EVOLUTION;
using namespace EVOLUTION::SOUND;

#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dsound.lib")

void EVOLUTION::FUNCTION::CreateSoundFactory(SOUND::ISoundFactory** pp_sound_factory, void *hwnd){
	*pp_sound_factory = nullptr;
	SoundFactory* sound_factory = NEW SoundFactory();
	sound_factory->Create((HWND)hwnd);
	*pp_sound_factory = sound_factory;
}

SoundFactory::SoundFactory(){

}

SoundFactory::~SoundFactory(){

}


u32 SoundFactory::AddRef(){
    return this->m_instance_counter.AddRef();
}

u32 SoundFactory::Release(){
    u32 counter = this->m_instance_counter.Release();
    if (counter == 0){
        delete this;
    }
    return counter;
}

RESULT SoundFactory::QueryInterface(EVOLUTION::EVOLUTION_IID riid, void **ppvObject){

    if (EVOLUTION_EQUALGUID(riid, EVOLUTION_GUID::IID_IUnknown))
    {
        *ppvObject = static_cast<IUnknown *>(this);
        this->AddRef();
    }
    else if (EVOLUTION_EQUALGUID(riid, EVOLUTION_GUID::IID_ISoundFactory))
    {
        *ppvObject = static_cast<ISoundFactory*>(this);
        this->AddRef();
    }
    else if (EVOLUTION_EQUALGUID(riid, EVOLUTION_GUID::IID_SoundFactory))
    {
        *ppvObject = static_cast<SoundFactory*>(this);
        this->AddRef();
    }
    else
    {
        *ppvObject = nullptr;
        return RESULT::E_no_instance;
    }
    return RESULT::S_ok;
}

void SoundFactory::Create(HWND hwnd){

	static struct _DS{
		WAVEFORMATEX m_primary_format;
		IDirectSound *mp_sound_device;
		IDirectSoundBuffer* mp_primary_buffer;
		_DS(HWND hwnd){
			HRESULT hr = S_OK;
			hr = DirectSoundCreate(NULL, &this->mp_sound_device, NULL);
            EVOLUTION_ASSERT(hr == S_OK);

			hr = this->mp_sound_device->SetCooperativeLevel(hwnd, DSSCL_EXCLUSIVE);
            EVOLUTION_ASSERT(hr == S_OK);

			DSBUFFERDESC dsdesc;
			ZeroMemory(&dsdesc, sizeof(DSBUFFERDESC));
			dsdesc.dwSize = sizeof(DSBUFFERDESC);
			dsdesc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;
			dsdesc.dwBufferBytes = 0;
			dsdesc.lpwfxFormat = NULL;

			hr = this->mp_sound_device->CreateSoundBuffer(&dsdesc, &this->mp_primary_buffer, NULL);
            EVOLUTION_ASSERT(hr == S_OK);


			this->m_primary_format.cbSize = sizeof(WAVEFORMATEX);
			this->m_primary_format.wFormatTag = WAVE_FORMAT_PCM;
			this->m_primary_format.nChannels = 2;
			this->m_primary_format.nSamplesPerSec = 44100;
			this->m_primary_format.wBitsPerSample = 16;
			this->m_primary_format.nBlockAlign = this->m_primary_format.nChannels * this->m_primary_format.wBitsPerSample / 8;
			this->m_primary_format.nAvgBytesPerSec = this->m_primary_format.nSamplesPerSec * this->m_primary_format.nBlockAlign;
			hr = this->mp_primary_buffer->SetFormat(&this->m_primary_format);

            EVOLUTION_ASSERT(hr == S_OK);
		}

		~_DS(){
            EVOLUTION_RELEASE(mp_primary_buffer);
            EVOLUTION_RELEASE(mp_sound_device);
		}

	}_DS(hwnd);

	this->mp_primary_buffer = _DS.mp_primary_buffer;
	this->mp_sound_device = _DS.mp_sound_device;
	this->m_primary_format = _DS.m_primary_format;
}

void SoundFactory::CreateStreamSoundManager(IStreamSoundManager** pp_stream_sound_manager){
	*pp_stream_sound_manager = nullptr;
	StreamSoundManager* streamsoundmanager = NEW StreamSoundManager(this->mp_sound_device, &this->m_primary_format);
    EVOLUTION_ASSERT_LOG(streamsoundmanager);
	*pp_stream_sound_manager = streamsoundmanager;
}

void SoundFactory::CreateSound(ISound** pp_sound, ISoundFileLoader* sound_file){
	*pp_sound = nullptr;
	Sound* sound = NEW Sound(this->mp_sound_device , &this->m_primary_format);
    EVOLUTION_ASSERT(sound != nullptr);
	sound->Create(sound_file);
	*pp_sound = sound;
}

void SoundFactory::CreateWriteSound(IWriteSound** pp_write_sound, u32 buffer_size, const WAVEFORMATEX& sound_format){
    *pp_write_sound = nullptr;
    WriteSound* sound = NEW WriteSound(this->mp_sound_device, &this->m_primary_format);
    EVOLUTION_ASSERT(sound != nullptr);
    sound->Create(buffer_size, sound_format);
    *pp_write_sound = sound;
}

void SoundFactory::CreateSound3D(ISound3D** pp_sound3d, ISoundFileLoader* sound_file){
	*pp_sound3d = nullptr;
	Sound3D* sound3d = NEW Sound3D(this->mp_sound_device, &this->m_primary_format);
	EVOLUTION_ASSERT(sound3d != nullptr);
	sound3d->Create(sound_file);
	*pp_sound3d = sound3d;
}


void SoundFactory::CreateSoundFile(ISoundFileLoader** p_sound_file, const char* file_name){
    *p_sound_file = nullptr;

    std::string extension = CORE::FILE::File::GetExtension(file_name);

    if (extension.compare(".wav") == 0){
        SoundFile_WavLoader* wav = NEW SoundFile_WavLoader(file_name);
        *p_sound_file = wav;
    }
}