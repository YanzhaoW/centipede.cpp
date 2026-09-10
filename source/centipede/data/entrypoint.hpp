#pragma once

#include "centipede/data/ValueError.hpp"
#include "centipede/data/entrypoint_base.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iterator>
#include <ranges>
#include <utility>
#include <vector>

namespace centipede
{
    /**
     * @brief Structure of a entrypoint with dynamic sizes.
     *
     * Values of local and global derivative are store in `std::vector`. Values are added to the vector via the setters
     * or #add_local and #add_global methods. It's highly recommended to reserve the memory first for the underlying
     * data to avoid the memory reallocation.
     */
    template <std::size_t NLocals = internal::DYNAMIC_SIZE, std::size_t NGlobals = internal::DYNAMIC_SIZE>
    class EntryPoint : public internal::EntryPointBase
    {
      public:
        using LocalDerivs = std::vector<ValueError<data_type>>; //!< Data type of local deriv array.
        using GlobalDerivs =
            std::vector<std::pair<uint32_t, ValueError<data_type>>>; //!< Data type of global deriv array.

        /**
         * @brief Default constructor.
         *
         * All values are initialized to the default values.
         *
         */
        EntryPoint() = default;

        /**
         * @brief Getter for local parameter sizes.
         * @return Sizes of local parameters.
         */
        [[nodiscard]] inline auto get_n_locals() const -> std::size_t { return locals_.size(); }

        /**
         * @brief Getter for global parameter sizes.
         * @return Sizes of global parameters.
         */
        [[nodiscard]] inline auto get_n_globals() const -> std::size_t { return globals_.size(); }

        /**
         * @brief Add a value to the local derivatives.
         * @param value_error Local derivative value with its uncertainty.
         * @return Non-const reference to this object.
         */
        constexpr auto add_local(internal::ValueErrorConvertible<data_type> auto value_error) -> auto&
        {
            locals_.emplace_back(value_error);
            return *this;
        }

        /**
         * @brief Add an index-value pair to the global derivatives.
         * @param index Global parameter ID (0-based indexing).
         * @param value Global derivative value.
         * @param error Uncertainty of global derivative value.
         * @return Non-const reference to this object.
         */
        constexpr auto add_global(std::integral auto index, internal::ValueErrorConvertible<data_type> auto value)
            -> auto&
        {
            globals_.emplace_back(static_cast<uint32_t>(index), static_cast<data_type>(value));
            return *this;
        }

        using internal::EntryPointBase::set_globals;
        using internal::EntryPointBase::set_locals;

        /**
         * @brief Set global derivative values from a view.
         *
         * The view must reference a std::pair, which must satisfy EntryPointGlobalIdxPair concept. The global derivate
         * values are emptied before being set by the new values.
         *
         * @tparam View Types of the view.
         * @param view View object.
         * @return Universal reference to the caller.
         */
        template <std::ranges::input_range View>
            requires EntryPointGlobalIdxPair<std::ranges::range_value_t<View>, data_type>
        constexpr auto set_globals(View view) -> auto&&
        {
            globals_.clear();
            std::ranges::copy(
                view | std::views::transform(
                           [](const auto& idx_ele)
                           { return std::pair{ idx_ele.first, ValueError<data_type>{ idx_ele.second } }; }),
                std::back_inserter(globals_));
            return *this;
        }

        /**
         * @brief Set local derivative values from a view.
         *
         * The view must reference a value of DataType. The local derivate values are emptied before being set by the
         * new values.
         *
         * @tparam View Types of the view.
         * @param view View object.
         * @return Universal reference to the caller.
         */
        template <typename View>
            requires internal::ValueErrorConvertible<std::ranges::range_value_t<View>, data_type>
        constexpr auto set_locals(View view) -> auto&&
        {
            locals_.clear();
            std::ranges::copy(
                view | std::views::transform([](const auto& ele) { return ValueError<data_type>{ ele }; }),
                std::back_inserter(locals_));
            return *this;
        }

        /**
         * @brief Set the capacity of the vector storing the local derivatives
         * @param size New capacity value.
         * @return Non-const reference to this object.
         */
        constexpr auto reserve_locals(std::size_t size) -> auto&
        {
            locals_.reserve(size);
            return *this;
        }

        /**
         * @brief Set the capacity of the vector storing the global derivatives
         * @param size New capacity value.
         * @return Non-const reference to this object.
         */
        constexpr auto reserve_globals(std::size_t size) -> auto&
        {
            globals_.reserve(size);
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

        template <internal::ValueErrorConvertible<data_type>... DataTypes>
        constexpr void set_locals_imp(DataTypes... locals)
        {
            locals_.clear();
            (locals_.push_back(ValueError<data_type>(locals)), ...);
        }

        template <EntryPointGlobalIdxPair<data_type>... DataTypes>
        constexpr void set_globals_imp(DataTypes... globals)
        {
            globals_.clear();
            (globals_.emplace_back(static_cast<uint32_t>(globals.first), ValueError<data_type>(globals.second)), ...);
        }

        constexpr void reset_derivs()
        {
            locals_.clear();
            globals_.clear();
        }
    };

    /**
     * @brief Structure of a entrypoint with static sizes.
     *
     * Entrypoint contains the derivatives of local and global parameters, together with measurement and sigma
     * values. The values are stored locally.
     * @anchor NLocals_DY_Entrypoint
     * @tparam NLocals Number of local parameters.
     * @anchor NGlobals_DY_Entrypoint
     * @tparam NGlobals Number of global parameters.
     */
    template <std::size_t NLocals, std::size_t NGlobals>
        requires(NLocals < internal::DYNAMIC_SIZE and NGlobals < internal::DYNAMIC_SIZE)
    class EntryPoint<NLocals, NGlobals> : public internal::EntryPointBase
    {
      public:
        using LocalDerivs = std::array<ValueError<data_type>, NLocals>; //!< Data type of local deriv array.
        using GlobalDerivs =
            std::array<std::pair<uint32_t, ValueError<data_type>>, NGlobals>; //!< Data type of global deriv array.

        /**
         * @brief Default constructor.
         *
         * All values are initialized to the default values.
         *
         * @see Config
         */
        EntryPoint() = default;

        using internal::EntryPointBase::set_globals;
        using internal::EntryPointBase::set_locals;

        /**
         * @brief Getter for local parameter sizes.
         * @return Sizes of local parameters.
         */
        [[nodiscard]] inline auto get_n_locals() const -> std::size_t { return NLocals; }

        /**
         * @brief Getter for global parameter sizes.
         * @return Sizes of global parameters.
         */
        [[nodiscard]] inline auto get_n_globals() const -> std::size_t { return NGlobals; }

        /**
         * @brief Default spaceship comparison.
         */
        constexpr auto operator<=>(const EntryPoint<NLocals, NGlobals>& other) const = default;

        /**
         * @brief Set global derivative values from a view.
         *
         * The view must reference a std::pair, which must satisfy EntryPointGlobalIdxPair concept. The size of the
         * input view is assumed to be larger or equal to @ref NGlobals_DY_Entrypoint "NGlobals".
         *
         * @tparam View Types of the view.
         * @param view View object.
         * @return Universal reference to the caller.
         */
        template <std::ranges::input_range View>
            requires EntryPointGlobalIdxPair<std::ranges::range_value_t<View>, data_type>
        constexpr auto set_globals(View view) -> auto&&
        {
            std::ranges::copy(view | std::views::take(NGlobals), globals_.begin());
            return *this;
        }

        /**
         * @brief Set local derivative values from a view.
         *
         * The view must reference a value of DataType.  The size of the input view is assumed to be larger or equal
         * to
         * @ref NLocals_DY_Entrypoint "NLocals".
         *
         * @tparam View Types of the view.
         * @param view View object.
         * @return Universal reference to the caller.
         */
        template <typename View>
            requires internal::ValueErrorConvertible<std::ranges::range_value_t<View>, data_type>
        constexpr auto set_locals(View view) -> auto&&
        {
            std::ranges::copy(view | std::views::take(NLocals), locals_.begin());
            return *this;
        }

      private:
        friend EntryPointBase;
        LocalDerivs locals_{};   //!< Local derivatives.
        GlobalDerivs globals_{}; //!< Global label and derivatives pair. The label is using **0-based indexing**.

        template <internal::ValueErrorConvertible<data_type>... DataTypes>
        constexpr void set_locals_imp(DataTypes... locals)
        {
            locals_ = std::array{ ValueError<data_type>(locals)... };
        }

        template <EntryPointGlobalIdxPair<data_type>... DataTypes>
        constexpr void set_globals_imp(DataTypes... globals)
        {
            globals_ = std::array{ std::pair{ static_cast<uint32_t>(globals.first),
                                              ValueError<data_type>(globals.second) }... };
        }

        constexpr void reset_derivs()
        {
            locals_ = std::array<ValueError<data_type>, NLocals>{};
            globals_ = std::array<std::pair<uint32_t, ValueError<data_type>>, NGlobals>{};
        }
    };
}; // namespace centipede

/**
 * @brief Formatter for the class EntryPoint
 */
template <std::size_t NLocals, std::size_t NGlobals>
// NOLINTNEXTLINE (bugprone-std-namespace-modification)
struct std::formatter<centipede::EntryPoint<NLocals, NGlobals>>
{
    static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    static auto format(const centipede::EntryPoint<NLocals, NGlobals>& entry, std::format_context& ctx)
    {
        return std::format_to(
            ctx.out(),
            "local derivatives: {}, global derivatives: {}, measurement: {}",
            entry.get_locals() | std::views::transform([](const auto& val_err) { return std::format("{}", val_err); }) |
                std::views::join_with(','),
            entry.get_globals() |
                std::views::transform(
                    [](const auto& idx_val_err)
                    {
                        const auto& [idx, val_err] = idx_val_err;
                        return std::format("{{{}: {}}}", idx, val_err);
                    }) |
                std::views::join_with(','),
            entry.get_measurement());
    }
};
