#pragma once

#include "centipede/data/entry_base.hpp"
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <format>
#include <utility>
#include <vector>

namespace centipede
{
    /**
     * @brief Structure of a entrypoint.
     *
     * Entrypoint contains the derivatives of local and global parameters, together with measurement and sigma values.
     * The values are stored locally.
     * @tparam NLocals Number of local parameters.
     * @tparam NGlobals Number of global parameters.
     */
    template <std::size_t NLocals = internal::DYNAMIC_SIZE, std::size_t NGlobals = internal::DYNAMIC_SIZE>
    class EntryPoint : public internal::EntryPointBase
    {
        using LocalDerivs = std::vector<float>;                       //!< Data type of local deriv array.
        using GlobalDerivs = std::vector<std::pair<uint32_t, float>>; //!< Data type of global deriv array.

        /**
         * @brief Default constructor.
         *
         * All values are initialized to the default values.
         *
         */
        EntryPoint() = default;

        /**
         * @brief Add a value to the local derivatives.
         * @param value Local derivative value.
         * @return Non-const reference to this object.
         */
        constexpr auto add_local(std::floating_point auto value) -> auto&
        {
            locals_.push_back(static_cast<float>(value));
            return *this;
        }

        /**
         * @brief Add an index-value pair to the global derivatives.
         * @param index Global parameter ID (0-based indexing).
         * @param value Global derivative value.
         * @return Non-const reference to this object.
         */
        constexpr auto add_global(std::integral auto index, std::floating_point auto value) -> auto&
        {
            globals_.emplace_back(static_cast<uint32_t>(index), static_cast<float>(value));
            return *this;
        }

        /**
         * @brief Default spaceship comparison.
         */
        constexpr auto operator<=>(const EntryPoint<NLocals, NGlobals>& other) const = default;

      private:
        friend EntryPointBase;
        LocalDerivs locals_;   //!< Local derivatives.
        GlobalDerivs globals_; //!< Global label and derivatives pair. The label is using **0-based indexing**.

        template <std::floating_point... DataTypes>
        constexpr void set_locals_imp(DataTypes... locals)
        {
            locals_.clear();
            (locals_.push_back(locals), ...);
        }

        template <EntryPointGlobalIdxPair... DataTypes>
        constexpr void set_globals_imp(DataTypes... globals)
        {
            locals_.clear();
            (globals_.emplace_back(static_cast<uint32_t>(globals.first), static_cast<float>(globals.second)), ...);
        }

        constexpr void reset_derivs()
        {
            locals_.clear();
            globals_.clear();
        }
    };

    /**
     * @brief Structure of a entrypoint.
     *
     * Entrypoint contains the derivatives of local and global parameters, together with measurement and sigma values.
     * The values are stored locally.
     * @tparam NLocals Number of local parameters.
     * @tparam NGlobals Number of global parameters.
     */
    template <std::size_t NLocals, std::size_t NGlobals>
        requires(NLocals < internal::DYNAMIC_SIZE and NGlobals < internal::DYNAMIC_SIZE)
    class EntryPoint<NLocals, NGlobals> : public internal::EntryPointBase
    {
      public:
        using LocalDerivs = std::array<float, NLocals>;                        //!< Data type of local deriv array.
        using GlobalDerivs = std::array<std::pair<uint32_t, float>, NGlobals>; //!< Data type of global deriv array.
        static constexpr std::size_t n_locals = NLocals;                       //!< Number of the local parameters.
        static constexpr std::size_t n_globals = NGlobals;                     //!< Number of the global parameters.

        /**
         * @brief Default constructor.
         *
         * All values are initialized to the default values.
         *
         * @see Config
         */
        EntryPoint() = default;

        /**
         * @brief Default spaceship comparison.
         */
        constexpr auto operator<=>(const EntryPoint<NLocals, NGlobals>& other) const = default;

      private:
        friend EntryPointBase;
        LocalDerivs locals_{};   //!< Local derivatives.
        GlobalDerivs globals_{}; //!< Global label and derivatives pair. The label is using **0-based indexing**.

        template <std::floating_point... DataTypes>
        constexpr void set_locals_imp(DataTypes... locals)
        {
            locals_ = std::array{ static_cast<float>(locals)... };
        }

        template <EntryPointGlobalIdxPair... DataTypes>
        constexpr void set_globals_imp(DataTypes... globals)
        {
            globals_ =
                std::array{ std::pair{ static_cast<uint32_t>(globals.first), static_cast<float>(globals.second) }... };
        }

        constexpr void reset_derivs()
        {
            locals_ = std::array<float, NLocals>{};
            globals_ = std::array<std::pair<uint32_t, float>, NGlobals>{};
        }
    };

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
                              entry.get_locals(),
                              entry.get_globals(),
                              entry.get_measurement(),
                              entry.get_sigma());
    }
};
