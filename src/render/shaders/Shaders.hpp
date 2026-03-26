#pragma once
#include <map>
static const std::map<std::string, std::string> SHADERS = {
{"blur1.frag",
#include "./blur1.frag.inc"
},
{"blur2.frag",
#include "./blur2.frag.inc"
},
{"blurfinish.frag",
#include "./blurfinish.frag.inc"
},
{"blurprepare.frag",
#include "./blurprepare.frag.inc"
},
{"border.frag",
#include "./border.frag.inc"
},
{"CM.frag",
#include "./CM.frag.inc"
},
{"CM.glsl",
#include "./CM.glsl.inc"
},
{"ext.frag",
#include "./ext.frag.inc"
},
{"glitch.frag",
#include "./glitch.frag.inc"
},
{"passthru.frag",
#include "./passthru.frag.inc"
},
{"quad.frag",
#include "./quad.frag.inc"
},
{"rgba.frag",
#include "./rgba.frag.inc"
},
{"rgbamatte.frag",
#include "./rgbamatte.frag.inc"
},
{"rgbx.frag",
#include "./rgbx.frag.inc"
},
{"rounding.glsl",
#include "./rounding.glsl.inc"
},
{"shadow.frag",
#include "./shadow.frag.inc"
},
{"tex300.vert",
#include "./tex300.vert.inc"
},
{"tex320.vert",
#include "./tex320.vert.inc"
},
};
