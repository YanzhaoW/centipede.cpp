#pragma once

#include <cstddef>
#include <optional>
#include <ranges>
#include <set>
#include <unordered_map>
#include <utility>

#if HAS_LIBASSERT
#include <assert.hpp>
#else
#include <cassert>
#endif

namespace centipede::core
{
    /**
     * @brief Map to to convert between the global parameter indices and unfixed global parameter indices.
     *
     */
    class ParIdMap
    {
      public:
        /**
         * @brief Default constructor
         *
         * @param n_pars Total number of global parameters.
         * @param fixed_pars A set of indices of fixed global parameters.
         */
        ParIdMap(std::size_t n_pars, const std::set<std::size_t>& fixed_pars = {})
        {
            for (const auto [par_id, unfixed_par_id] : std::views::zip_transform(
                     [](const auto unfixed_par_id, const auto par_id) -> std::pair<std::size_t, std::size_t>
                     { return std::pair{ par_id, unfixed_par_id }; },
                     std::views::iota(0UZ, n_pars - fixed_pars.size()),
                     std::views::iota(0UZ, n_pars) | std::views::filter([&fixed_pars](const auto par_id) -> bool
                                                                        { return not fixed_pars.contains(par_id); })))
            {
                unfixed_par_id_map_.try_emplace(par_id, unfixed_par_id);
                unfixed_par_id_inverse_map_.try_emplace(unfixed_par_id, par_id);
            }
        }

        /**
         * @brief Get the unfixed global parameter index from global parameter index.
         *
         * @param par_id Global parameter index.
         * @return Unfixed parameter index.
         */
        auto get_unfixed_par_id(std::size_t par_id) const -> std::optional<std::size_t>
        {
            if (auto unfixed_par_iter = unfixed_par_id_map_.find(par_id); unfixed_par_iter != unfixed_par_id_map_.end())
            {
                return unfixed_par_iter->second;
            }
            return {};
        }

        /**
         * @brief Get the global parameter index from the unfixed parameter index.
         *
         * @param par_id Unfixed global parameter index.
         * @return Global parameter index.
         */
        auto get_par_id(std::size_t par_id) const -> std::size_t
        {
            auto par_iter = unfixed_par_id_inverse_map_.find(par_id);
#if HAS_LIBASSERT
            debug_assert(par_iter != unfixed_par_id_inverse_map_.end());
#else
            assert(par_iter != unfixed_par_id_inverse_map_.end());
#endif
            return par_iter->second;
        }

        /**
         * @brief Get the map with the global parameter indices as the keys and unfixed indices as the values.
         *
         * @return Map of indices
         */
        [[nodiscard]] auto get_unfixed_par_id_map() const -> const auto& { return unfixed_par_id_map_; }

      private:
        using IdMap = std::unordered_map<std::size_t, std::size_t>;
        IdMap unfixed_par_id_map_;         //!< A map with global parameter ID as the key and unfixed global
                                           //!< parameter ID as value.
        IdMap unfixed_par_id_inverse_map_; //!< A map with unfixed global parameter ID as the key and global
                                           //!< parameter ID as value.
    };
} // namespace centipede::core
