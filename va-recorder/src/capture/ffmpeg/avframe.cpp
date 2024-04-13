#include "avframe.h"

namespace va {
    void release_avframe(AVFrame *frame) {
        av_frame_free(&frame);
    }
} // namespace va