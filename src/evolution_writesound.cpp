#include "evolution_soundimplement.h"

using namespace EVOLUTION;
using namespace EVOLUTION::SOUND;

u32 WriteSound::AddRef(){
    return this->m_instance_counter.AddRef();
}

u32 WriteSound::Release(){
    u32 counter = this->m_instance_counter.Release();
    if (counter == 0){
        EVOLUTION_RELEASE(this->mp_soundbuffer);
        delete this;
    }
    return counter;
}

RESULT WriteSound::QueryInterface(EVOLUTION::EVOLUTION_IID riid, void **ppvObject){

    if (IsEqualGUID(riid, EVOLUTION_GUID::IID_IUnknown))
    {
        *ppvObject = static_cast<IUnknown *>(this);
        this->AddRef();
    }
    else if (IsEqualGUID(riid, EVOLUTION_GUID::IID_IWriteSound))
    {
        *ppvObject = static_cast<IWriteSound*>(this);
        this->AddRef();
    }
    else if (IsEqualGUID(riid, EVOLUTION_GUID::IID_ISound))
    {
        *ppvObject = static_cast<ISound*>(this);
        this->AddRef();
    }
    else if (IsEqualGUID(riid, EVOLUTION_GUID::IID_WriteSound))
    {
        *ppvObject = static_cast<WriteSound*>(this);
        this->AddRef();
    }
    else
    {
        *ppvObject = nullptr;
        return _RESULT::E_no_instance;
    }
    return _RESULT::S_ok;
}

WriteSound::WriteSound(IDirectSound* direct_sound, WAVEFORMATEX* primary_format) :mp_sound_device(direct_sound), mp_primary_format(primary_format){

}

WriteSound::~WriteSound(){

}

RESULT WriteSound::Create(u32 buffer_size, const WAVEFORMATEX& sound_format){

    this->m_buffer_size = buffer_size;

    DSBUFFERDESC dsdesc;
    ZeroMemory(&dsdesc, sizeof(DSBUFFERDESC));
    dsdesc.dwSize = sizeof(DSBUFFERDESC);
    dsdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_STICKYFOCUS;
    dsdesc.guid3DAlgorithm = DS3DALG_DEFAULT;

    dsdesc.dwBufferBytes = buffer_size;
    WAVEFORMATEX waveformatex = sound_format;
    dsdesc.lpwfxFormat = &waveformatex;

    HRESULT hr;
    hr = this->mp_sound_device->CreateSoundBuffer(&dsdesc, &this->mp_soundbuffer, NULL);
    EVOLUTION_ASSERT(hr == S_OK);

    return _RESULT::S_ok;
}

//サウンドのコピー
ISound* WriteSound::Copy(){
    return nullptr;
}

void WriteSound::Play(bool loop){
    this->mp_soundbuffer->Play(0, 0, (loop) ? DSBPLAY_LOOPING : 0);
}

void WriteSound::Pause(){
    this->mp_soundbuffer->Stop();
}

void WriteSound::Stop(){
    this->mp_soundbuffer->Stop();
    this->mp_soundbuffer->SetCurrentPosition(0);
}

void WriteSound::SetPan(s32 pan){
    this->mp_soundbuffer->SetPan(pan);
}

void WriteSound::SetVolume(s32 volume){
    this->mp_soundbuffer->SetVolume(volume);
}

void WriteSound::SetPlayPos(u32 second){
    this->mp_soundbuffer->SetCurrentPosition(second * this->mp_primary_format->nAvgBytesPerSec);
}

void WriteSound::SetFrequency(f32 frequency){
    u32 f = (u32)(this->mp_primary_format->nSamplesPerSec * frequency);
    this->mp_soundbuffer->SetFrequency(f);
}

s32 WriteSound::GetVolume(){
    s32 vol;
    this->mp_soundbuffer->GetVolume((LPLONG)&vol);
    return vol;
}

f32 WriteSound::GetFrequency(){
    u32 fer;
    this->mp_soundbuffer->GetFrequency((LPDWORD)&fer);
    return (float)fer / this->mp_primary_format->nSamplesPerSec;
}

u32 WriteSound::GetPlayPosSecond(){
    u32 pos;
    this->mp_soundbuffer->GetCurrentPosition((LPDWORD)&pos, NULL);
    return pos / this->mp_primary_format->nAvgBytesPerSec;
}

u32 WriteSound::GetPlayPosMillSecond(){
    u32 pos;
    this->mp_soundbuffer->GetCurrentPosition((LPDWORD)&pos, NULL);
    return pos / (this->mp_primary_format->nAvgBytesPerSec / 10);
}

u32 WriteSound::GetPlayPosBuffer(){
    u32 pos;
    this->mp_soundbuffer->GetCurrentPosition((LPDWORD)&pos, NULL);
    return pos;
}

u32 WriteSound::GetTime(){
    return this->m_max_time;
}

bool WriteSound::IsPlay(){
    u32 state;
    this->mp_soundbuffer->GetStatus((LPDWORD)&state);
    return state & DSBSTATUS_PLAYING;
}

//バッファサイズの取得
u32 WriteSound::GetBufferSize()const{
    return this->m_buffer_size;
}

//バッファを書き込む
void WriteSound::WriteBuffer(u32 write_position, void* buffer, s32 buffer_size){
    // ロック開始
    LPVOID pMem1, pMem2;
    u32 dwSize1, dwSize2;

    HRESULT hr = this->mp_soundbuffer->Lock(write_position, buffer_size, &pMem1, (LPDWORD)&dwSize1, &pMem2, (LPDWORD)&dwSize2, 0);
    EVOLUTION_ASSERT(hr == S_OK);

    u8* work_ptr = (u8*)buffer;

    // データ書き込み
    memcpy(pMem1, &work_ptr, dwSize1);
    memcpy(pMem2, &work_ptr + dwSize1, dwSize2);

    // ロック解除
    hr = this->mp_soundbuffer->Unlock(pMem1, dwSize1, pMem2, dwSize2);
}