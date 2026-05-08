#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace centipede
{
    /**
     * @class Entry
     * @brief A structure to store the all entrypoints in a entry.
     *
     * Values in the Entrypoints are stored in the corresponding vectors in order. Each element in the measurement or
     * sigma vector represent the value of one entrypoints. However, different elements in the local derivs or global
     * derivs vector can belong to the same entrypoint. Thus, elements in those vectors have the type #Deriv,
     * which is a pair of the entrypoint ID and index-value pair.
     */
    template <typename DataType>
    struct Entry
    {
        using Deriv = std::pair<uint32_t, std::pair<uint32_t, DataType>>; //!< The key of the outer pair is the
                                                                          //!< entrypoint ID and the key of the inner
                                                                          //!< pair is the index of the parameter.
        std::optional<std::size_t> n_locals; //!< Cache to store the temporary local parameter size
        std::vector<DataType> measurements;  //!< Measurements from all entrypoints.
        std::vector<DataType> sigmas;        //!< Sigmas from all entrypoints.
        std::vector<Deriv> local_derivs;     //!< Local derivatives.
        std::vector<Deriv> global_derivs;    //!< Global derivatives.
    };

}; // namespace centipede
