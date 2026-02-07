#pragma once

#include "centipede/util/common_traits.hpp"
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <ranges>
#include <utility>

namespace centipede
{
    /**
     * @brief Concept of a type that is convertible to global index value pair
     *
     * @tparam T Type of a std::pair.
     */
    template <typename T>
    concept EntryPointGlobalIdxPair = requires(T pair) {
        requires internal::IsPair<T>::value;
        requires std::convertible_to<typename internal::IsPair<T>::first_type, uint32_t>;
        requires std::is_integral_v<typename internal::IsPair<T>::first_type>;
        requires std::convertible_to<typename internal::IsPair<T>::second_type, float>;
        requires std::is_floating_point_v<typename internal::IsPair<T>::second_type>;
    };

} // namespace centipede

namespace centipede::internal
{
    constexpr auto DYNAMIC_SIZE = std::numeric_limits<std::size_t>::max();

    /**
     * @brief Base class for all entrypoint classes.
     *
     * Entrypoint contains the derivatives of local and global parameters, together with measurement and sigma values.
     * There are two derived classes: `%EntryPoint` (all using default template parameters) and
     * `%EntryPoint<NLocals, NGlobals>`. The first derived class stores the data in std::vector via the dynamic
     * allocations, whereas the second derived class stores the data in std::array via the static allocation.
     *
     * Each derived class must have this base class as `friend`, such that the base class can access the private member
     * variables from the getters. The design principle of querying the member variables in the derived classes is
     * [_deducing
     * this_](https://en.cppreference.com/w/cpp/language/member_functions.html#Explicit_object_member_functions) in C++
     * 23 (see get_globals() or get_locals()), which is very similar to _CRTP_ in pre-23 standards.
     *
     * Return values from the setters are of the type `auto&&` which is induced from
     * `std::forward<decltype(auto)>(self)`, which enables the possibility to chain the setters all together. This also
     * makes sure that if `self` is a l-value, the return value is l-value reference and if `self` is a r-value, it
     * returns the original type through RVO.
     */
    class EntryPointBase
    {
      protected:
        /**
         * @brief Protected default constructor.
         *
         * This constructor can only be accessed through its derived classes.
         *
         */
        EntryPointBase() = default;

      public:
        using data_type = float; //!< Data type of deriv values.

        //
        /**
         * @brief Reset all values to the default values.
         *
         * In this base class, only member variables #measurement_ and #sigma_ are resetted to their default values.
         * `reset_derivs` method implemented from derived classes is called to reset the derivative values.
         * @param self The dynamic reference to the caller.
         * @return Universal reference to the caller.
         */
        constexpr auto reset(this auto&& self) -> auto&&
        {
            self.reset_derivs();
            self.measurement_ = float{};
            self.sigma_ = float{};
            return std::forward<decltype(self)>(self);
        }

        /**
         * @brief Set derivative values of local parameters.
         *
         * The input arguments are passed to the `set_locals_imp` method, which is implemented from the derived
         * classes.
         *
         * Example:
         * ```cpp
         * entry_point.set_locals(1.1, 2.3, 3.2);
         * ```
         *
         * @tparam DataTypes Types of derivative values.
         * @param self The dynamic reference to the caller.
         * @param locals Derivative values of local parameters.
         * @return Universal reference to the caller.
         */
        template <std::floating_point... DataTypes>
        constexpr auto set_locals(this auto&& self, DataTypes... locals) -> auto&&
        {
            self.set_locals_imp(std::forward<DataTypes>(locals)...);
            return std::forward<decltype(self)>(self);
        }

        /**
         * @brief Set derivative values of local parameters from a generator.
         *
         * The generator must take no argument and return a floating point value.
         * @tparam Gen Types of the generator.
         * @param self The dynamic reference to the caller.
         * @param generator Generator.
         * @return Universal reference to the caller.
         */
        template <typename Gen>
            requires requires(Gen generator) {
                { generator() } -> std::floating_point<>;
            }
        constexpr auto set_locals(this auto&& self, Gen generator) -> auto&&
        {
            std::ranges::generate(self.locals_, generator);
            return std::forward<decltype(self)>(self);
        }

        /**
         * @brief Set derivative values of global parameters.
         *
         * The input arguments are passed to the `set_globals_imp` method, which is implemented from the derived
         * classes.
         *
         * Example:
         * ```cpp
         * entry_point.set_globals(std::pair{0, 3.}, std::pair{2, 7.});
         * ```
         *
         * @tparam DataTypes Types of derivative values.
         * @param self The dynamic reference to the caller.
         * @param globals Derivative values.
         * @return Universal reference to the caller.
         * @see EntryPointGlobalIdxPair
         */
        template <EntryPointGlobalIdxPair... DataTypes>
        constexpr auto set_globals(this auto&& self, DataTypes... globals) -> auto&&
        {
            self.set_globals_imp(std::forward<DataTypes>(globals)...);
            return std::forward<decltype(self)>(self);
        }

        /**
         * @brief Set global derivative values from a generator.
         *
         * The generator must take no argument and return a std::pair, which must satisfy EntryPointGlobalIdxPair
         * concept.
         *
         * @tparam Gen Types of the generator.
         * @param self The dynamic reference to the caller.
         * @param generator Generator.
         * @return Universal reference to the caller.
         */
        template <typename Gen>
            requires requires(Gen generator) {
                { generator() } -> EntryPointGlobalIdxPair<>;
            }
        constexpr auto set_globals(this auto&& self, Gen generator) -> auto&&
        {
            std::ranges::generate(self.globals_, generator);
            return std::forward<decltype(self)>(self);
        }

        /**
         * @brief Set global derivative values from two generators.
         *
         * The first generator must take no argument and return a integral value, while the second one takes no argument
         * and returns a floating point value.
         * @tparam IdxGen Types of the generator for global indices.
         * @tparam ValGen Types of the generator for global derivative values.
         * @param self The dynamic reference to the caller.
         * @param idx_gen Generator for indices.
         * @param val_gen Generator for values.
         * @return Universal reference to the caller.
         */
        template <typename IdxGen, typename ValGen>
            requires requires(IdxGen idx_gen, ValGen val_gen) {
                { idx_gen() } -> std::integral<>;
                { val_gen() } -> std::floating_point<>;
            }
        constexpr auto set_globals(this auto&& self, IdxGen idx_gen, ValGen val_gen) -> auto&&
        {
            std::ranges::generate(self.globals_ | std::views::values, idx_gen);
            std::ranges::generate(self.globals_ | std::views::keys, val_gen);
            return std::forward<decltype(self)>(self);
        }

        /**
         * @brief Set the measurement of the current entry point.
         * @param self The dynamic reference to the caller.
         * @param value Measurement value.
         * @return Universal reference to the caller.
         */
        constexpr auto set_measurement(this auto&& self, std::floating_point auto value) -> auto&&
        {
            self.measurement_ = static_cast<float>(value);
            return std::forward<decltype(self)>(self);
        }

        /**
         * @brief Set the sigma of the current entry point.
         * @param self The dynamic reference to the caller.
         * @param value Sigma value.
         * @return Universal reference to the caller.
         */
        constexpr auto set_sigma(this auto&& self, std::floating_point auto value) -> auto&&
        {
            self.sigma_ = static_cast<float>(value);
            return std::forward<decltype(self)>(self);
        }

        /**
         * @brief Getter for local derivatives.
         * @param self The dynamic reference to the caller.
         * @return Const reference to the local derivatives.
         */
        [[nodiscard]] auto get_locals(this auto&& self) -> const auto& { return self.locals_; }

        /**
         * @brief Getter for global derivatives.
         * @param self The dynamic reference to the caller.
         * @return Const reference to the global derivatives.
         */
        [[nodiscard]] auto get_globals(this auto&& self) -> const auto& { return self.globals_; }

        /**
         * @brief Getter for the measurement value.
         * @return Value of the measurement.
         */
        [[nodiscard]] auto get_measurement() const -> float { return measurement_; }

        /**
         * @brief Getter for the sigma value.
         * @return Value of the sigma.
         */
        [[nodiscard]] auto get_sigma() const -> float { return sigma_; }

        /**
         * @brief Default spaceship comparison.
         */
        constexpr auto operator<=>(const EntryPointBase& other) const = default;

      private:
        float measurement_{}; //!< Measurement value.
        float sigma_{};       //!< Error value.
    };

} // namespace centipede::internal
