#include "MediaDecoder.h"

MediaDecoder::MediaDecoder(const Napi::CallbackInfo &info)
    : AsyncWorker(info.Env()),
      ObjectWrap<MediaDecoder>(info)
{
  av_log_set_level(AV_LOG_INFO);
  av_log(NULL, AV_LOG_INFO, "avlog is working.\n");
  Napi::Env env = info.Env();
  if (info.Length() < 1 || !info[0].IsFunction())
  {
    av_log(NULL, AV_LOG_INFO, "expected one function as argument.\n");
    Napi::TypeError::New(env, "expected one function as argument.\n");
    return;
  }
  Napi::Function jsEmit = info[0].As<Napi::Function>();
  tsFun = Napi::ThreadSafeFunction::New(env, jsEmit, "tsFun", 0, 1);
}
void MediaDecoder::setInputUrl(const Napi::CallbackInfo &info)
{
  if (info[0].IsString())
  {
    inputUrl = info[0].As<Napi::String>().Utf8Value().c_str();
    av_log(NULL, AV_LOG_INFO, "setInputUrl %s.\n", inputUrl);
  }
}
void MediaDecoder::setInputFormat(const Napi::CallbackInfo &info)
{
  if (info[0].IsString())
  {
    inputFmtName = info[0].As<Napi::String>().Utf8Value().c_str();
    inputFmt = av_find_input_format(inputFmtName);
    av_log(NULL, AV_LOG_INFO, "setInputFormat %s.\n", inputFmtName);
    if (inputFmt == NULL)
    {
      av_log(NULL, AV_LOG_ERROR, "av_find_input_format failed!!!\n");
    }
  }
}
void MediaDecoder::setInputOption(const Napi::CallbackInfo &info)
{
  if (info.Length() >= 2 && info[0].IsString() && info[1].IsString())
  {
    const char *key = info[0].As<Napi::String>().Utf8Value().c_str();
    const char *val = info[1].As<Napi::String>().Utf8Value().c_str();
    av_dict_set(&inputOptions, key, val, 0);
    av_log(NULL, AV_LOG_INFO, "setInputOption %s:%s.\n", key, val);
  }
}
void MediaDecoder::openInput(const Napi::CallbackInfo &info)
{
  // 打开输入流并读取头
  // Open an input stream and read the header.
  if (ret = avformat_open_input(&inputFmtCtx, inputUrl, inputFmt, &inputOptions))
  {
    av_log(NULL, AV_LOG_FATAL, "open inputUrl %s faild.\n", inputUrl);
    return;
  }
  // 打印输入信息
  // Print detailed information about the input
  av_dump_format(inputFmtCtx, 0, inputUrl, 0);
}
void MediaDecoder::initDecoder(const Napi::CallbackInfo &info)
{
  // 探测流信息
  // Read packets of a media file to get stream information.
  if ((ret = avformat_find_stream_info(inputFmtCtx, NULL)) < 0)
  {
    av_log(NULL, AV_LOG_ERROR, "avformat_find_stream_info failed");
    return cleanUp();
  }
  // 得到视频流
  videoIdx = av_find_best_stream(inputFmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
  // 得到音频流
  audioIdx = av_find_best_stream(inputFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
  // 至少得到一个视频流或音频流
  if (videoIdx < 0 && audioIdx < 0)
  {
    av_log(NULL, AV_LOG_ERROR, "av_find_best_stream failed");
    return cleanUp();
  }
  if (videoIdx >= 0)
  {
    // 得到编解码上下文
    if ((videoDecoderCtx = avcodec_alloc_context3(NULL)) == NULL)
    {
      av_log(NULL, AV_LOG_ERROR, "avcodec_alloc_context3 failed");
      return cleanUp();
    }
    // 得到编解码上下文
    if ((videoDecoderCtx = avcodec_alloc_context3(NULL)) == NULL)
    {
      av_log(NULL, AV_LOG_ERROR, "avcodec_alloc_context3 failed");
      return cleanUp();
    }
    // 将视频流编解码器信息拷贝到编解码上下文
    if ((ret = avcodec_parameters_to_context(videoDecoderCtx, inputFmtCtx->streams[videoIdx]->codecpar)) < 0)
    {
      av_log(NULL, AV_LOG_ERROR, "avcodec_parameters_to_context failed");
      return cleanUp();
    }

    // 根据编解码器id获取解码器
    const AVCodec *videoDecoder = avcodec_find_decoder(videoDecoderCtx->codec_id);
    if (videoDecoder == NULL)
    {
      av_log(NULL, AV_LOG_ERROR, "avcodec_find_decoder failed");
      return cleanUp();
    }

    // 使用 编解码器 初始化 编解码器的上下文
    if ((ret = avcodec_open2(videoDecoderCtx, videoDecoder, NULL)) < 0)
    {
      av_log(NULL, AV_LOG_ERROR, "avcodec_open2 failed");
      return cleanUp();
    }
  }
  if (audioIdx >= 0)
  {
    // 得到编解码上下文
    if ((audioDecoderCtx = avcodec_alloc_context3(NULL)) == NULL)
    {
      av_log(NULL, AV_LOG_ERROR, "avcodec_alloc_context3 failed");
      return cleanUp();
    }
    // 得到编解码上下文
    if ((audioDecoderCtx = avcodec_alloc_context3(NULL)) == NULL)
    {
      av_log(NULL, AV_LOG_ERROR, "avcodec_alloc_context3 failed");
      return cleanUp();
    }
    // 将视频流编解码器信息拷贝到编解码上下文
    if ((ret = avcodec_parameters_to_context(audioDecoderCtx, inputFmtCtx->streams[audioIdx]->codecpar)) < 0)
    {
      av_log(NULL, AV_LOG_ERROR, "avcodec_parameters_to_context failed");
      return cleanUp();
    }
    // 根据编解码器id获取解码器
    const AVCodec *audioDecoder = avcodec_find_decoder(audioDecoderCtx->codec_id);
    if (audioDecoder == NULL)
    {
      av_log(NULL, AV_LOG_ERROR, "avcodec_find_decoder failed");
      return cleanUp();
    }
    // 使用 编解码器 初始化 编解码器的上下文
    if ((ret = avcodec_open2(audioDecoderCtx, audioDecoder, NULL)) < 0)
    {
      av_log(NULL, AV_LOG_ERROR, "avcodec_open2 failed");
      return cleanUp();
    }
  }
}
void MediaDecoder::decode(const Napi::CallbackInfo &info)
{
  this->Queue();
}
void MediaDecoder::Execute()
{
  // 读取流
  AVPacket *packet = av_packet_alloc();
  while (av_read_frame(inputFmtCtx, packet) == 0)
  {
    // std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
    // 如果是视频流
    if (packet->stream_index == videoIdx)
    {
      // 把packet发送给解码器得到YUV帧
      // 将数据发送给解码器
      if ((ret = avcodec_send_packet(videoDecoderCtx, packet)) == 0)
      {
        AVFrame *videoFrame = NULL; // 一定要在每次接收都重新分配内存，然后在线程安全函数中释放内存
        // 接收解码器解码后的数据frame
        while (avcodec_receive_frame(videoDecoderCtx, (videoFrame = av_frame_alloc())) == 0)
        {
          // YUV420P
          // Y
          // 【Y Y Y Y】
          // 【Y Y Y Y】
          // U
          // 【U 0 U 0】
          // 【0 0 0 0】
          // V
          // 【V 0 V 0】
          // 【0 0 0 0】
          tsFun.BlockingCall(
              [this, videoFrame](Napi::Env env, Napi::Function jsEmiter)
              {
                try
                {
                  Napi::String pix_fmt = Napi::String::New(env, av_get_pix_fmt_name(videoDecoderCtx->pix_fmt));
                  Napi::Number width = Napi::Number::New(env, videoDecoderCtx->width);
                  Napi::Number height = Napi::Number::New(env, videoDecoderCtx->height);
                  // YUV420P
                  Napi::Buffer Y = Napi::Buffer<uint8_t>::NewOrCopy(env, videoFrame->data[0], videoFrame->linesize[0] * videoDecoderCtx->height);
                  Napi::Buffer U = Napi::Buffer<uint8_t>::NewOrCopy(env, videoFrame->data[1], videoFrame->linesize[1] * videoDecoderCtx->height / 2);
                  Napi::Buffer V = Napi::Buffer<uint8_t>::NewOrCopy(env, videoFrame->data[2], videoFrame->linesize[2] * videoDecoderCtx->height / 2);
                  jsEmiter.Call({
                      Napi::String::New(env, "video"),
                      Y,
                      U,
                      V,
                      width,
                      height,
                      pix_fmt,
                  });
                }
                catch (const Napi::Error &e)
                {
                  // 处理错误
                  e.ThrowAsJavaScriptException();
                }
                av_frame_unref(videoFrame); // 释放内存
              });
        }
      }
    }
    if (packet->stream_index == audioIdx)
    {
      // 把packet发送给解码器得到PCM帧
      // 将数据发送给解码器
      if ((ret = avcodec_send_packet(audioDecoderCtx, packet)) == 0)
      {
        // 接收解码器解码后的数据frame
        AVFrame *audioFrame = NULL; // 一定要在每次接收都重新分配内存，然后在线程安全函数中释放内存
        while (avcodec_receive_frame(audioDecoderCtx, audioFrame = av_frame_alloc()) == 0)
        {
          /**
           * audioFrame  格式：fltp(float32-le)
           * 非交错格式（Planer）
           * frame->data[ch=0] L L L L
           * frame->data[ch=1] R R R R
           */
          tsFun.BlockingCall(
              [this, audioFrame](Napi::Env env, Napi::Function jsEmiter)
              {
                try
                {
                  Napi::Array PCM = Napi::Array::New(env, audioFrame->ch_layout.nb_channels);
                  Napi::String sample_fmt = Napi::String::New(env, av_get_sample_fmt_name(audioDecoderCtx->sample_fmt));
                  Napi::Number sample_bytes = Napi::Number::New(env, av_get_bytes_per_sample(audioDecoderCtx->sample_fmt));
                  Napi::Number nb_samples = Napi::Number::New(env, audioFrame->nb_samples);
                  Napi::Number nb_channels = Napi::Number::New(env, audioFrame->ch_layout.nb_channels);
                  for (uint8_t chIdx = 0; chIdx < audioFrame->ch_layout.nb_channels; chIdx++)
                    PCM.Set(chIdx, Napi::Buffer<uint8_t>::NewOrCopy(env, audioFrame->data[chIdx], av_get_bytes_per_sample(audioDecoderCtx->sample_fmt) * audioFrame->nb_samples));
                  jsEmiter.Call({
                      Napi::String::New(env, "audio"),
                      PCM,
                      nb_samples,
                      nb_channels,
                      sample_bytes,
                      sample_fmt,
                  });
                }
                catch (const Napi::Error &e)
                {
                  // 处理错误
                  e.ThrowAsJavaScriptException();
                }
                av_frame_unref(audioFrame); // 释放内存
              });
        }
      }
    }
    av_packet_unref(packet);
  }
  //
}
void MediaDecoder::OnOK()
{
  std::cout << "OnOK" << std::endl;
}
void MediaDecoder::Destroy()
{
  std::cout << "Destroy" << std::endl;
}
void MediaDecoder::OnError(const Napi::Error &e)
{
  std::cout << "OnError" << std::endl;
}

void MediaDecoder::cleanUp()
{
  if (inputFmtCtx)
    avformat_close_input(&inputFmtCtx);
  if (videoDecoderCtx)
    avcodec_free_context(&videoDecoderCtx);
  if (audioDecoderCtx)
    avcodec_free_context(&audioDecoderCtx);
}

Napi::Object MediaDecoder::Init(Napi::Env env, Napi::Object exports)
{
  Napi::Function MediaDecoderClassConstructerFun =
      DefineClass(
          env,
          "MediaDecoder",
          {
              InstanceMethod("setInputUrl", &MediaDecoder::setInputUrl),
              InstanceMethod("setInputFormat", &MediaDecoder::setInputFormat),
              InstanceMethod("setInputOption", &MediaDecoder::setInputOption),
              InstanceMethod("openInput", &MediaDecoder::openInput),
              InstanceMethod("initDecoder", &MediaDecoder::initDecoder),
              InstanceMethod("decode", &MediaDecoder::decode),
          });
  // 保证构造函数引用不被GC
  Napi::FunctionReference *funRef = new Napi::FunctionReference();
  *funRef = Napi::Persistent(MediaDecoderClassConstructerFun);
  env.SetInstanceData(funRef);
  exports.Set("MediaDecoder", MediaDecoderClassConstructerFun);
  return exports;
}
