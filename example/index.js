const { MediaDecoder } = require("media-decoder")

let canvas = document.querySelector("#canvas")
let mediaDecoder = new MediaDecoder();

mediaDecoder.setInputUrl("rtsp://localhost:8554/mystream")
// mediaDecoder.setInputFormat("rtsp")
// mediaDecoder.setInputOption()
mediaDecoder.openInput()
mediaDecoder.initDecoder()
mediaDecoder.decode()
mediaDecoder.setupCanvas(canvas)