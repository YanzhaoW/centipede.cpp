#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <utility>

namespace centipede
{
    /**
     * @class centipede::EntryPoint
     * @brief Structure of a entrypoint.
     *
     * Entrypoint contains the derivatives of local and global parameters, together with measurement and sigma values.
     * The values are stored locally.
     */
    template <std::size_t NLocals, std::size_t NGlobals>
    struct EntryPoint
    {
        using data_type = float;                                               //!< Data type of deriv values.
        using LocalDerivs = std::array<float, NLocals>;                        //!< Data type of local deriv array.
        using GlobalDerivs = std::array<std::pair<uint32_t, float>, NGlobals>; //!< Data type of global deriv array.
        static constexpr std::size_t n_locals = NLocals;                       //!< Number of the local parameters.
        static constexpr std::size_t n_globals = NGlobals;                     //!< Number of the global parameters.

        // NOLINTBEGIN (misc-non-private-member-variables-in-classes)
        LocalDerivs local_derivs{};   //!< Local derivatives.
        GlobalDerivs global_derivs{}; //!< Global label and derivatives pair. The label is using **0-based indexing**.
        float measurement{};          //!< Measurement corresponding to the error value.
        float sigma{};                //!< Error value.
        // NOLINTEND (misc-non-private-member-variables-in-classes)

        /**
         * @brief Reset all values to the default values.
         */
        constexpr void reset()
        {
            local_derivs = std::array<float, NLocals>{};
            global_derivs = std::array<std::pair<uint32_t, float>, NGlobals>{};
            measurement = float{};
            sigma = float{};
        }

        /**
         * @brief Default spaceship operator for comparisons.
         */
        constexpr auto operator<=>(const EntryPoint<NLocals, NGlobals>& other) const = default;
    };

    /**
     * @brief Explicit CTAD deduction guide for centipede::EntryPoint
     */
    template <std::size_t NLocals, std::size_t NGlobals>
    EntryPoint(std::array<float, NLocals>, std::array<std::pair<uint32_t, float>, NGlobals>, float, float)
        -> EntryPoint<NLocals, NGlobals>;
}; // namespace centipede

/**
 * @brief Formatter for the class EntryPoint
 */
template <std::size_t NLocals, std::size_t NGlobals>
struct std::formatter<centipede::EntryPoint<NLocals, NGlobals>>
{
    static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    static auto format(const centipede::EntryPoint<NLocals, NGlobals>& entry, std::format_context& ctx)
    {
        return std::format_to(ctx.out(),
                              "local derivatives: {}, global derivatives: {}, measurement: {}, sigma: {}",
                              entry.local_derivs,
                              entry.global_derivs,
                              entry.measurement,
                              entry.sigma);
    }
};
