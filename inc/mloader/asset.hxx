#pragma once

enum Type {
    INVALID = 0,  // Invalid type value
    BINARY,       // Raw binary blob
    IMAGE,
    SHADER,       // GLSL, HLSL, or similar shader code
    SOUND,
    FONT,
    TEXT,         // utf-8 text
};

enum State {
    UNLOADED   = 0,
    UNPARSED   = 1,
    PARSED     = 2,
};
