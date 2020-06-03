#pragma once

#include <queue>
#include <sstream>

#ifdef PILI_DEBUG_ENABLED
#   define PILI_DEBUG_ADDTEXT(x) debug().addText() << x
#   define PILI_DEBUG_NEWLINE() debug().addLine()
#else
#   define PILI_DEBUG_ADDTEXT(x) do {} while (false)
#   define PILI_DEBUG_NEWLINE() do {} while (false)
#endif

namespace pipeliner {

    class Debug {
    public:
        Debug() { addLine(); }

        std::stringstream &addText() { return lines_.back(); }

        void addLine() { lines_.push({}); }

        std::string popLine() {
            if (lines_.size()) {
                auto l = lines_.front().str();
                lines_.pop();
                return l;
            } else {
                return {};
            }
        }

    private:
        std::queue<std::stringstream> lines_;
    };

}
