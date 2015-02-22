#include "evolution_soundimplement.h"

using namespace EVOLUTION;
using namespace EVOLUTION::SOUND;

//SHA1用HASHサイズ
#define HASH_SIZE 21

StreamSound::StreamSound(StreamSoundManager* stream_sound_manager, IDirectSound* direct_sound, WAVEFORMATEX* primary_format) :mp_sound_device(direct_sound), mp_primary_format(primary_format), mp_tmp_file(nullptr), mp_stream_sound_manager(stream_sound_manager){

}

StreamSound::~StreamSound(){

}

u32 StreamSound::AddRef(){
    return this->m_instance_counter.AddRef();
}

u32 StreamSound::Release(){
    u32 counter = this->m_instance_counter.Release();
    if (counter == 0){
        mp_stream_sound_manager->Delete(this);
        EVOLUTION_RELEASE(this->mp_soundbuffer);
        EVOLUTION_RELEASE(this->mp_tmp_file);
        delete this;
    }
    return counter;
}

RESULT StreamSound::QueryInterface(EVOLUTION::EVOLUTION_IID riid, void **ppvObject){

    if (EVOLUTION_EQUALGUID(riid, EVOLUTION_GUID::IID_IUnknown))
    {
        *ppvObject = static_cast<IUnknown *>(this);
        this->AddRef();
    }
    else if (EVOLUTION_EQUALGUID(riid, EVOLUTION_GUID::IID_IStreamSound))
    {
        *ppvObject = static_cast<IStreamSound*>(this);
        this->AddRef();
    }
    else if (EVOLUTION_EQUALGUID(riid, EVOLUTION_GUID::IID_StreamSound))
    {
        *ppvObject = static_cast<StreamSound*>(this);
        this->AddRef();
    }
    else
    {
        *ppvObject = nullptr;
        return RESULT::E_no_instance;
    }
    return RESULT::S_ok;
}

void StreamSound::Create(ISoundFileLoader* soundfile){
    //サウンドデータのコピー
    TEMPLATE::Array<u8> buffer(soundfile->GetSoundBufferSize());
    //サウンドのコピー
    buffer.Set(soundfile->GetSoundBuffer(), soundfile->GetSoundBufferSize());

    //一時ファイルに書き出し
    auto crypto = EVOLUTION::CORE::Hash::GetInstance();
    u8 tmp_file_name[HASH_SIZE];
    crypto->SHA1(tmp_file_name, &buffer[0], soundfile->GetSoundBufferSize());
    std::string tmp;
 
    tmp.reserve(FUNCTION::Strlen(CONFIG::Sound_Temp_Dir) + HASH_SIZE);
    tmp += CONFIG::Sound_Temp_Dir;
    CORE::FILE::Directory::Create(tmp.c_str());
    for (s32 i = 0; i < HASH_SIZE - 1; i++)
    {
        char c_16[2];
        FUNCTION::BitToChar(c_16, tmp_file_name[i]);
        tmp += c_16[0];
        tmp += c_16[1];
    }

    if (!CORE::FILE::File::IsCheck(tmp.c_str())){
        CORE::FILE::IFileWrite* tmp_file_writer;
        FUNCTION::CreateFileWrite(&tmp_file_writer, tmp.c_str(), CORE::FILE::Mode::_NEW);
        //コピー
        buffer.Set(soundfile->GetSoundBuffer(), soundfile->GetSoundBufferSize());
        tmp_file_writer->Write(&buffer[0], buffer.GetMaxCount());
        tmp_file_writer->Close();
        EVOLUTION_RELEASE(tmp_file_writer);
    }



    //一時ファイルのオープン
    EVOLUTION::FUNCTION::CreateFileRead(&this->mp_tmp_file, tmp.c_str());
    //タイムの取得
    this->m_max_time = soundfile->GetSoundBufferSize() / soundfile->GetSoundFormat().nAvgBytesPerSec;


    DSBUFFERDESC dsdesc;
    ZeroMemory(&dsdesc, sizeof(DSBUFFERDESC));
    dsdesc.dwSize = sizeof(DSBUFFERDESC);
    dsdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_STICKYFOCUS;
    dsdesc.guid3DAlgorithm = DS3DALG_DEFAULT;
    //フォーマットの取得
    WAVEFORMATEX waveformatex = soundfile->GetSoundFormat();
    dsdesc.lpwfxFormat = &waveformatex;
    dsdesc.dwBufferBytes = soundfile->GetSoundBufferSize();

    if (this->m_max_time > CONFIG::Stream_Sec){
        //バッファ数 = channel * サンプリング数 * bitrate(16bit(2byte) or 24bit(3byte)) * 秒数
        dsdesc.dwBufferBytes = waveformatex.nChannels * waveformatex.nSamplesPerSec * (waveformatex.wBitsPerSample / 8) * CONFIG::Stream_Sec;
    }
    HRESULT hr;
    hr = this->mp_sound_device->CreateSoundBuffer(&dsdesc, &this->mp_soundbuffer, NULL);
   EVOLUTION_ASSERT_LOG(hr == S_OK);

    //バッファのサイズ
    this->m_buffer_size = dsdesc.dwBufferBytes;

    // ロック開始
    LPVOID pMem1, pMem2;
    u32 dwSize1, dwSize2;
    hr = this->mp_soundbuffer->Lock(0, this->m_buffer_size, &pMem1, (LPDWORD)&dwSize1, &pMem2, (LPDWORD)&dwSize2, 0);
    EVOLUTION_ASSERT_LOG(hr == S_OK);

    // データ書き込み
    this->mp_tmp_file->Read(&pMem1, dwSize1);
    this->mp_tmp_file->Read(&pMem2, dwSize2);

    // ロック解除
    hr = this->mp_soundbuffer->Unlock(pMem1, dwSize1, pMem2, dwSize2);
    EVOLUTION_ASSERT_LOG(hr == S_OK);

    this->m_load_buffer_number = (this->m_buffer_size / (this->m_buffer_size / 2)) % 2;

    mp_stream_sound_manager->Add(this);
}


IStreamSound* StreamSound::Copy(){
    EVOLUTION_ASSERT(0);
    return nullptr;
}

void StreamSound::Play(bool loop){
    this->m_loop = loop;
    this->mp_soundbuffer->Play(0, 0, DSBPLAY_LOOPING);
}

void StreamSound::Pause(){
    this->mp_soundbuffer->Stop();
    this->mp_soundbuffer->SetCurrentPosition(0);
}

void StreamSound::Stop(){
    this->mp_soundbuffer->Stop();
}

void StreamSound::SetPan(s32 pan){
    this->mp_soundbuffer->SetPan(pan);
}

void StreamSound::SetVolume(s32 volume){
    this->mp_soundbuffer->SetVolume(volume);
}

void StreamSound::SetPlayPos(u32 second){
    EVOLUTION_ASSERT(0);
    this->mp_soundbuffer->SetCurrentPosition(second * this->mp_primary_format->nAvgBytesPerSec);
}

void StreamSound::SetFrequency(float frequency){
    u32 f = (u32)(this->mp_primary_format->nSamplesPerSec * frequency);
    this->mp_soundbuffer->SetFrequency(f);
}

s32 StreamSound::GetVolume(){
    s32 vol;
    this->mp_soundbuffer->GetVolume((LPLONG)&vol);
    return vol;
}

float StreamSound::GetFrequency(){
    u32 fer;
    this->mp_soundbuffer->GetFrequency((LPDWORD)&fer);
    return (float)fer / this->mp_primary_format->nSamplesPerSec;
}

u32 StreamSound::GetPlayPosSecond(){
    EVOLUTION_ASSERT(0);
    u32 pos;
    this->mp_soundbuffer->GetCurrentPosition((LPDWORD)&pos, NULL);
    return pos / this->mp_primary_format->nAvgBytesPerSec;
}

u32 StreamSound::GetPlayPosMillSecond(){
    EVOLUTION_ASSERT(0);
    u32 pos;
    this->mp_soundbuffer->GetCurrentPosition((LPDWORD)&pos, NULL);
    return pos / (this->mp_primary_format->nAvgBytesPerSec / 10);
}

u32 StreamSound::GetPlayPosBuffer(){
    u32 pos;
    this->mp_soundbuffer->GetCurrentPosition((LPDWORD)&pos, NULL);
    return pos;
}

u32 StreamSound::GetTime(){
    return this->m_max_time;
}

bool StreamSound::IsPlay(){
    u32 state;
    this->mp_soundbuffer->GetStatus((LPDWORD)&state);
    return state & DSBSTATUS_PLAYING;
}

bool StreamSound::IsLoop(){
    return this->m_loop;
}

void StreamSound::LoadStream(){
    u32 buffer_size = this->m_buffer_size / 2;
    u32 pos = (this->GetPlayPosBuffer() / buffer_size) % 2;

    if (m_load_buffer_number == pos){
        return;
    }

    // ロック開始
    u8 *pMem1, *pMem2;
    u32 dwSize1, dwSize2;

    //１秒分のバッファのロック
    auto hr = this->mp_soundbuffer->Lock(m_load_buffer_number * buffer_size, buffer_size, (void**)&pMem1, (LPDWORD)&dwSize1, (void**)&pMem2, (LPDWORD)&dwSize2, 0);
    EVOLUTION_ASSERT_LOG(hr == S_OK);

    u32 read_data_size1 = dwSize1, read_data_size2 = dwSize2;
    // データ書き込み
    if (dwSize1){
        read_data_size1 = this->mp_tmp_file->Read(pMem1, dwSize1);
    }
    if (dwSize2){
        read_data_size2 = this->mp_tmp_file->Read(pMem2, dwSize2);
    }

    if (dwSize1 != read_data_size1){
        if (this->m_loop){
            this->mp_tmp_file->SetPos(0);
            read_data_size1 = this->mp_tmp_file->Read(pMem1 + read_data_size1, dwSize1 - read_data_size1) + read_data_size1;
        }
        else{
            if (read_data_size1 == 0){
                this->Stop();
            }
            memset(pMem1 + read_data_size1, 0, dwSize1 - read_data_size1);
        }
    }

    if (dwSize2 != read_data_size2){
        if (this->m_loop){
            this->mp_tmp_file->SetPos(0);
            read_data_size2 = this->mp_tmp_file->Read(pMem2 + read_data_size2, dwSize2 - read_data_size2) + read_data_size2;
        }
        else{
            memset(pMem1 + read_data_size1, 0, dwSize1 - read_data_size1);
        }
    }

    // ロック解除
    hr = this->mp_soundbuffer->Unlock(pMem1, dwSize1, pMem2, dwSize2);
    EVOLUTION_ASSERT_LOG(hr == S_OK);

    (this->m_load_buffer_number % 2 == 1) ? this->m_load_buffer_number = 0 : this->m_load_buffer_number = 1;
}