#pragma once

#include "centipede/data/entry.hpp"
#include "centipede/util/return_types.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace centipede::core::engines
{
    /**
     * @brief Engine types.
     *
     */
    enum class MatrixEngineType : uint8_t
    {
        eigen,
        xtensor,
        blaze
    };

    template <MatrixEngineType, typename DataType>
    class Engine
    {
    };

    template <MatrixEngineType engine_type, typename DataType>
    class Master
    {
      public:
        template <std::size_t NLocals, std::size_t NGlobals>
        [[nodiscard]] auto add_entrypoint(const EntryPoint<NLocals, NGlobals>& entry_point) -> EnumError<>
        {
            current_entry_.measurements.push_back(entry_point.get_measurement());
            current_entry_.sigmas.push_back(entry_point.get_sigma());
            std::ranges::copy(entry_point.get_locals(), std::back_inserter(current_entry_.local_derivs));
            std::ranges::copy(entry_point.get_globals(), std::back_inserter(current_entry_.global_derivs));
        }

        void analyze() { engine_imp_.fill_data(current_entry_); }

      private:
        Entry<DataType> current_entry_;
        Engine<engine_type, DataType> engine_imp_;

        std::vector<DataType> global_vals_;
    };

    /**
     * @brief Base engine classes.
     *
     */
    template <typename DataType>
    class Base
    {
      public:
        void fill_data(this auto&& self, const Entry<DataType>& entry)
        {
            std::ranges::copy(entry.measurements, std::back_inserter(self.measurements_));
            std::ranges::copy(entry.sigmas, std::back_inserter(self.sigmas_));
            self.fill_local_derivs(entry.local_derivs);
            self.fill_global_derivs(entry.local_derivs);
        }

        auto get_current_entrypoint_size() const -> auto
        {
            assert(measurements_.size() == sigmas_.size());
            return measurements_.size();
        }

      private:
        Base() = default;

        std::vector<DataType> measurements_;
        std::vector<DataType> sigmas_;

        // auto get_current_entry() const -> const auto& { return current_entry_; }
    };

} // namespace centipede::core::engines
