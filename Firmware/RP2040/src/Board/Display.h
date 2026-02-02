#pragma once

namespace Display {
    void initialize();
    void enable(bool en);
    void printf(const char* fmt, ...);
    void render_now();
}
