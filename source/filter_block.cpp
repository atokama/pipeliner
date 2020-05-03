#include <pipeliner/filter_block.h>

namespace pipeliner {

    const std::list<double> FilterBlock::mul_ =
            {0.00025177, 0.008666992, 0.078025818,
             0.24130249, 0.343757629, 0.24130249,
             0.078025818, 0.008666992, 0.000125885};

}