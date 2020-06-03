#include <queue>
#include <string>

namespace pipeliner {

    class Debug {
    public:
        Debug() : enabled_{false} {}

        void enable() {
            enabled_ = true;
            addLine();
        }

        void addText(const std::string &s) {
            if (enabled_) {
                lines_.back() += s;
            }
        }

        void addLine() {
            if (enabled_) {
                lines_.push({});
            }
        }

        std::string popLine() {
            if (lines_.size()) {
                auto l = lines_.front();
                lines_.pop();
                return l;
            } else {
                return {};
            }
        }

    private:
        bool enabled_;
        std::queue<std::string> lines_;
    };

}
