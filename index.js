const addon = require('bindings')('MediaDecoder');
const { EventEmitter } = require("node:events")
const { setupCanvas, renderFrame } = require("./player.js")

class MediaDecoder extends EventEmitter {
    self = null
    constructor() {
        super();
        this.self = new addon.MediaDecoder(this.emit.bind(this))
    }
    setInputUrl(url) {
        this.self.setInputUrl(url)
    }
    setInputFormat(mft) {
        this.self.setInputFormat(mft)
    }
    setInputOption(key, val) {
        this.self.setInputOption(key, val)
    }
    openInput() {
        this.self.openInput()
    }
    initDecoder() {
        this.self.initDecoder()
    }
    decode() {
        this.self.decode()
    }
    setupCanvas(canvas) {
        const webgl_ctx = setupCanvas(canvas)
        this.once("video", (Y, U, V, width, height, pix_fmt) => {
            canvas.width = width
            canvas.height = height
        })
        this.on("video", (Y, U, V, width, height, pix_fmt) => {
            renderFrame(webgl_ctx, Y, U, V, width, height)
        })
    }
}

module.exports = {
    MediaDecoder
}