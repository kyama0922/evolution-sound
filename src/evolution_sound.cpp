#include "evolution_soundimplement.h"

using namespace EVOLUTION;
using namespace EVOLUTION::SOUND;

Sound::Sound(IDirectSound* direct_sound, WAVEFORMATEX* primary_format) :mp_sound_device(direct_sound), mp_primary_format(primary_format){

}

Sound::~Sound(){

}

u32 Sound::AddRef(){
    return this->m_instance_counter.AddRef();
}

u32 Sound::Release(){
    u32 counter = this->m_instance_counter.Release();
    if (counter == 0){
        EVOLUTION_RELEASE(this->mp_soundbuffer);
        delete this;
    }
    return counter;
}

RESULT Sound::QueryInterface(EVOLUTION::EVOLUTION_IID riid, void **ppvObject){

    if (EVOLUTION_EQUALGUID(riid, EVOLUTION_GUID::IID_IUnknown))
    {
        *ppvObject = static_cast<IUnknown *>(this);
        this->AddRef();
    }
    else if (EVOLUTION_EQUALGUID(riid, EVOLUTION_GUID::IID_ISound))
    {
        *ppvObject = static_cast<Sound*>(this);
        this->AddRef();
    }
    else if (EVOLUTION_EQUALGUID(riid, EVOLUTION_GUID::IID_Sound))
    {
        *ppvObject = static_cast<ISound*>(this);
        this->AddRef();
    }
    else
    {
        *ppvObject = nullptr;
        return RESULT::E_no_instance;
    }
    return RESULT::S_ok;
}

void Sound::Create(ISoundFileLoader* soundfile){

	DSBUFFERDESC dsdesc;
	ZeroMemory(&dsdesc, sizeof(DSBUFFERDESC));
	dsdesc.dwSize = sizeof(DSBUFFERDESC);
	dsdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_STICKYFOCUS;
	dsdesc.guid3DAlgorithm = DS3DALG_DEFAULT;

	dsdesc.dwBufferBytes = soundfile->GetSoundBufferSize();
	WAVEFORMATEX waveformatex = soundfile->GetSoundFormat();
	dsdesc.lpwfxFormat = &waveformatex;

	HRESULT hr;
	hr = this->mp_sound_device->CreateSoundBuffer(&dsdesc, &this->mp_soundbuffer, NULL);
	EVOLUTION_ASSERT(hr == S_OK);

	// ロック開始
	LPVOID pMem1, pMem2;
	u32 dwSize1, dwSize2;

	hr = this->mp_soundbuffer->Lock(0, soundfile->GetSoundBufferSize(), &pMem1, (LPDWORD)&dwSize1, &pMem2, (LPDWORD)&dwSize2, 0);
    EVOLUTION_ASSERT(hr == S_OK);

	//audioの長さを取得
	WAVEFORMATEX format;
	this->mp_soundbuffer->GetFormat((LPWAVEFORMATEX)&format, 0, nullptr);

	this->m_max_time = dwSize1 / format.nAvgBytesPerSec;

	TEMPLATE::Array<u8> buffer(soundfile->GetSoundBufferSize());
	//サウンドのコピー
    buffer.Set(soundfile->GetSoundBuffer(), soundfile->GetSoundBufferSize());
	
	// データ書き込み
	memcpy(pMem1, &buffer[0], dwSize1);
	memcpy(pMem2, &buffer[0] + dwSize1, dwSize2);

	// ロック解除
	hr = this->mp_soundbuffer->Unlock(pMem1, dwSize1, pMem2, dwSize2);
    EVOLUTION_ASSERT(hr == S_OK);

}


ISound* Sound::Copy(){

	Sound* work = new Sound(this->mp_sound_device, this->mp_primary_format);
	work->m_max_time = this->m_max_time;
	work->m_now_time = this->m_now_time;

	WAVEFORMATEX format;
	auto ht = this->mp_soundbuffer->GetFormat(&format, sizeof(WAVEFORMATEX), NULL);

	void* srcmem1, *srcmem2;
	u32 srcsize1, srcsize2;
	this->mp_soundbuffer->Lock(0, 0, &srcmem1, (LPDWORD)&srcsize1, &srcmem2, (LPDWORD)&srcsize2, DSBLOCK_ENTIREBUFFER);

	// サウンドバッファの設定
	DSBUFFERDESC dsdesc;
	ZeroMemory(&dsdesc, sizeof(DSBUFFERDESC));
	dsdesc.dwSize = sizeof(DSBUFFERDESC);
	dsdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_STICKYFOCUS;
	dsdesc.guid3DAlgorithm = DS3DALG_DEFAULT;

	dsdesc.dwBufferBytes = srcsize1 + srcsize2;
	dsdesc.lpwfxFormat = &format;

	this->mp_sound_device->CreateSoundBuffer(&dsdesc, &work->mp_soundbuffer, NULL);

	void* destmem1, *destmem2;
	u32 destsize1, destsize2;
	work->mp_soundbuffer->Lock(0, dsdesc.dwBufferBytes, &destmem1, (LPDWORD)&destsize1, &destmem2, (LPDWORD)&destsize2, 0);

	memcpy(destmem1, srcmem1, srcsize1);
	memcpy(destmem2, srcmem2, srcsize2);

	work->mp_soundbuffer->Unlock(destmem1, destsize1, destmem2, destsize2);
	this->mp_soundbuffer->Unlock(srcmem1, srcsize1, srcmem2, srcsize2);

	return work;
}

void Sound::Play(bool loop){
	this->mp_soundbuffer->Play(0, 0, (loop) ? DSBPLAY_LOOPING : 0);
}

void Sound::Pause(){
	this->mp_soundbuffer->Stop();
}

void Sound::Stop(){
	this->mp_soundbuffer->Stop();
	this->mp_soundbuffer->SetCurrentPosition(0);
}

void Sound::SetPan(s32 pan){
	this->mp_soundbuffer->SetPan(pan);
}

void Sound::SetVolume(s32 volume){
	this->mp_soundbuffer->SetVolume(volume);
}

void Sound::SetPlayPos(u32 second){
	this->mp_soundbuffer->SetCurrentPosition(second * this->mp_primary_format->nAvgBytesPerSec);
}

void Sound::SetFrequency(float frequency){
	u32 f = (u32)(this->mp_primary_format->nSamplesPerSec * frequency);
	this->mp_soundbuffer->SetFrequency(f);
}

s32 Sound::GetVolume(){
	s32 vol;
	this->mp_soundbuffer->GetVolume((LPLONG)&vol);
	return vol;
}

float Sound::GetFrequency(){
	u32 fer;
	this->mp_soundbuffer->GetFrequency((LPDWORD)&fer);
	return (float)fer / this->mp_primary_format->nSamplesPerSec;
}

u32 Sound::GetPlayPosSecond(){
	u32 pos;
	this->mp_soundbuffer->GetCurrentPosition((LPDWORD)&pos, NULL);
	return pos / this->mp_primary_format->nAvgBytesPerSec;
}

u32 Sound::GetPlayPosMillSecond(){
	u32 pos;
	this->mp_soundbuffer->GetCurrentPosition((LPDWORD)&pos, NULL);
	return pos / (this->mp_primary_format->nAvgBytesPerSec / 10);
}

u32 Sound::GetPlayPosBuffer(){
	u32 pos;
	this->mp_soundbuffer->GetCurrentPosition((LPDWORD)&pos, NULL);
	return pos;
}

u32 Sound::GetTime(){
	return this->m_max_time;
}

bool Sound::IsPlay(){
	u32 state;
	this->mp_soundbuffer->GetStatus((LPDWORD)&state);
	return state & DSBSTATUS_PLAYING;
}