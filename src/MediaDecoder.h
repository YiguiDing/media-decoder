// 除了 Node.js 之外，其他（Electron ）一些运行时已放弃对外部缓冲区的支持。
// 这意味着无法实现共享内存，Napi::Buffer::New(env,(uint8_t*)buffer,size)无法使用
// https://github.com/nodejs/node-addon-api/blob/main/doc/external_buffer.md
// https://github.com/electron/electron/issues/35801

#define NODE_API_NO_EXTERNAL_BUFFERS_ALLOWED

#include <napi.h>

#ifndef __MY_OBJECT__
#define __MY_OBJECT__

extern "C"
{
#include "libavutil/log.h"
#include "libavutil/pixelutils.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include <libavutil/pixdesc.h>
}

#include <iostream>
#include <thread>
#include <functional>

class MediaDecoder : public Napi::ObjectWrap<MediaDecoder>, public Napi::AsyncWorker
{
public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  MediaDecoder(const Napi::CallbackInfo &info);

private:
  int ret = 0;
  // for input
  const char *inputUrl = NULL;
  const char *inputFmtName = NULL;
  AVFormatContext *inputFmtCtx = NULL;
  const AVInputFormat *inputFmt = NULL;
  AVDictionary *inputOptions = NULL;
  // for decoder
  int videoIdx = -1;
  int audioIdx = -1;
  AVCodecContext *videoDecoderCtx = NULL;
  AVCodecContext *audioDecoderCtx = NULL;
  void setInputUrl(const Napi::CallbackInfo &info);
  void setInputFormat(const Napi::CallbackInfo &info);
  void setInputOption(const Napi::CallbackInfo &info);
  void openInput(const Napi::CallbackInfo &info);
  void initDecoder(const Napi::CallbackInfo &info);
  void decode(const Napi::CallbackInfo &info);
  void cleanUp();

  Napi::Function jsEmitt;
  Napi::ThreadSafeFunction tsFun;
  void Execute() override;
  void OnOK() override;
  void OnError(const Napi::Error &e) override;
  void Destroy() override;
};

#endif