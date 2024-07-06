#include "MediaDecoder.h"

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    MediaDecoder::Init(env, exports);
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)