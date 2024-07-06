{
  "targets": [
    {
      "target_name": "MediaDecoder",
      "defines": [
        "NODE_API_NO_EXTERNAL_BUFFERS_ALLOWED"
      ],
      "sources": [
        "src/Addon.cpp",
        "src/MediaDecoder.cpp",
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').targets\"):node_addon_api"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "ffmpeg/windows/ffmpeg-6.1.1-full_build-shared/include/"
      ],
      "library_dirs": [
        "ffmpeg/windows/ffmpeg-6.1.1-full_build-shared/lib/"
      ],
      "libraries": [
        "-lavutil",
        "-lavformat",
        "-lavcodec",
        "-lavdevice",
        "-lswscale"
      ]
    }
  ]
}