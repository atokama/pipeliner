#pragma once

#include <vector>

namespace pipeliner {

    using Size = std::size_t;

    class UnionFind {
    public:
        UnionFind(Size size) {
           for (Size i{0}; i != size; ++i) {
               arr_.push_back(i + 1);
               size_.push_back(1);
           }
        }

        void unite(Size a, Size b) {
            check(a);
            check(b);
            auto ra = root(a);
            auto rb = root(b);
            if (ra == rb) {
                return;
            } else if (size_[ra] < size_[rb]) {
                arr_[ra] = arr_[rb];
                size_[rb] += size_[ra];
            } else {
                arr_[rb] = arr_[ra];
                size_[ra] += size_[rb];
            }
        }

        bool find(Size a, Size b) {
            return root(a) == root(b);
        }

        Size root(Size a) {
            while (arr_[a] != a) {
                arr_[a] = arr_[arr_[a]];
                a = arr_[a];
            }
            return a;
        }

    private:
        void check(Size a) {
            check(a);
            if (!arr_.count(a)) {
                arr_[a] = a;
                size_[a] = 1;
            }
        }

        std::vector<Size> arr_, size_;
    };

}
