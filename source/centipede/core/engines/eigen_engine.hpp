#pragma once

#include "centipede/core/engines/base_engine.hpp"
#include "centipede/core/engines/engine_types.hpp"
#include "centipede/core/engines/result.hpp"
#include "centipede/data/entry.hpp"
#include "centipede/util/error_types.hpp"
#include "centipede/util/return_types.hpp"
#include <Eigen/Cholesky>
#include <Eigen/Core>
#include <Eigen/Eigenvalues>
#include <Eigen/Sparse>
#include <Eigen/SparseCore>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <expected>
#include <limits>
#include <ranges>
#include <utility>
#include <vector>

namespace centipede::core::engine
{
    /**
     * @brief Engine template specialization for Eigen library implementation.
     */
    template <typename DataType>
    class Engine<MatrixEngine::eigen, DataType> : public Base<DataType>
    {
      public:
        /**
         * @brief Matrix and vector used to solve global parameter updates
         */
        struct Globals
        {
            // TODO:  matrix is symmetric. Thus it's more efficient to represent it with a special memory layout.
            using MatrixType = Eigen::Matrix<DataType, Eigen::Dynamic, Eigen::Dynamic>;
            MatrixType factor_matrix{};
            Eigen::Matrix<DataType, Eigen::Dynamic, 1> rhs_vec{};
        };

        explicit Engine(std::size_t n_globals)
            : Base<DataType>(n_globals)
        {
            resize_globals(globals_, n_globals);
        }

        [[nodiscard]] auto get_local_solutions() const -> const auto& { return buffers_.local_solutions; };
        [[nodiscard]] auto get_global_factor_matrix() const -> const auto& { return globals_.factor_matrix; };
        [[nodiscard]] auto get_global_rhs_vector() const -> const auto& { return globals_.rhs_vec; };

        /**
         * @brief solve the updates of global parameters.
         */
        static void solve(const Globals& globals, Result<DataType>& result)
        {
            assert(globals.factor_matrix.isApprox(globals.factor_matrix.transpose()));

            if (globals.factor_matrix.isZero())
            {
                result.error_status = ErrorCode::analysis_factor_matrix_zero;
                return;
            }
            if (globals.rhs_vec.isZero())
            {
                result.error_status = ErrorCode::analysis_rhs_vector_zero;
                return;
            }

            auto cholesky_decomp = globals.factor_matrix.llt();

            if (cholesky_decomp.info() == Eigen::ComputationInfo::Success)
            {
                // NOTE: memory allocation here
                auto global_par_solution = cholesky_decomp.solve(globals.rhs_vec).eval();
                result.parameters.clear();
                std::ranges::copy(
                    std::views::zip_transform([](auto idx, const DataType& val) -> Result<DataType>::IdxValuePair
                                              { return typename Result<DataType>::IdxValuePair{ idx, val }; },
                                              std::views::iota(std::size_t{}),
                                              global_par_solution),
                    std::back_inserter(result.parameters));
                result.error_status = ErrorCode::success;
            }
            else
            {
                check_rank_deficit(globals, result);
            }
        }

        void add_to_globals(Globals& globals)
        {
            globals.factor_matrix += globals_.factor_matrix.eval();
            globals.rhs_vec += globals_.rhs_vec.eval();
        }

      private:
        constexpr static auto max_n_local = 20;
        using LocalRectangleMatrix =
            Eigen::Matrix<DataType, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor, max_n_local>;
        using LocalSquareMatrix =
            Eigen::Matrix<DataType, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor, max_n_local, max_n_local>;
        using LocalSquareVec = Eigen::Matrix<DataType, Eigen::Dynamic, 1, Eigen::ColMajor, max_n_local>;

        std::vector<Eigen::Triplet<DataType>> triplets_;
        LocalRectangleMatrix local_t_{}; //!< Transpose of the local derivs matrix. The row size is n_locals and
                                         //!< the column size is the number of entrypoints.
        Eigen::SparseMatrix<DataType> global_t_{}; //!< Transpose of the global derivs matrix. The row size is number of
                                                   //!< global parameters and column size is the number of entrypoints.
        Eigen::Matrix<DataType, Eigen::Dynamic, 1> sigmas_{};       //!< Sigma values
        Eigen::Matrix<DataType, Eigen::Dynamic, 1> measurements_{}; //!< Sigma values

        Globals globals_;

        struct
        {
            LocalRectangleMatrix local_weighted_t{};
            LocalSquareMatrix local_weighted_square{};
            LocalSquareMatrix local_weighted_square_inv{};
            LocalSquareVec local_weighted_meas{};
            Eigen::LLT<LocalSquareMatrix> cholesky_solver{ max_n_local };
            Eigen::Matrix<DataType, Eigen::Dynamic, 1> residual_values{};
            Eigen::Matrix<DataType, Eigen::Dynamic, 1> local_solutions{}; // Local solutions
            Eigen::SparseMatrix<DataType> local_weighted_square_inv_sparse{};
            Eigen::SparseMatrix<DataType> global_local_weighted_t{};
            Eigen::SparseMatrix<DataType> global_weighted_square{};
            Eigen::SparseMatrix<DataType> global_square_update{};
            Eigen::SparseMatrix<DataType> global_rhs_vector_update{};
            Eigen::SparseMatrix<DataType> sigmas_sparse_view{};
        } buffers_;

        friend Base<DataType>;

        void resize_buffers()
        {

            const auto& current_state = Base<DataType>::get_current_state();
            const auto entrypoint_size = current_state.n_points;
            const auto n_globals = current_state.n_globals;
            const auto n_locals = current_state.n_locals;

            // NOTE: resize may cause memory allocation.
            local_t_.resize(n_locals, entrypoint_size);
            local_t_.setZero();
            buffers_.local_weighted_t.resize(n_locals, entrypoint_size);
            buffers_.local_weighted_square.resize(n_locals, n_locals);
            buffers_.local_weighted_meas.resize(n_locals);
            buffers_.residual_values.resize(entrypoint_size);
            buffers_.local_weighted_square_inv.resize(n_locals, n_locals);
            buffers_.local_solutions.resize(n_locals);
            sigmas_.resize(entrypoint_size);
            measurements_.resize(entrypoint_size);

            // NOTE: resize initializes the sparse matrix to zero values
            global_t_.resize(n_globals, entrypoint_size);
            buffers_.global_local_weighted_t.resize(n_locals, n_globals);
            buffers_.global_weighted_square.resize(n_globals, n_globals);
            buffers_.global_square_update.resize(n_globals, n_globals);
            buffers_.local_weighted_square_inv_sparse.resize(n_locals, n_locals);
            buffers_.sigmas_sparse_view.resize(entrypoint_size, entrypoint_size);
        }

        void fill_sigmas(const std::vector<DataType>& data)
        {
            std::ranges::copy(std::views::transform(data, [](DataType val) -> DataType { return 1. / (val * val); }),
                              sigmas_.begin());
        }

        void fill_measurements(const std::vector<DataType>& data) { std::ranges::copy(data, measurements_.begin()); }

        void fill_local_derivs(const std::vector<typename Entry<DataType>::Deriv>& data)
        {
            for (const auto& [point_idx, deriv] : data)
            {
                assert(point_idx < local_t_.cols());
                assert(deriv.first < local_t_.rows());
                local_t_(deriv.first, point_idx) = deriv.second;
            }
        }

        void fill_global_derivs(const std::vector<typename Entry<DataType>::Deriv>& data)
        {
            triplets_.clear();

            for (const auto& [point_idx, deriv] : data)
            {
                assert(point_idx < global_t_.cols());
                assert(deriv.first < global_t_.rows());
                triplets_.emplace_back(deriv.first, point_idx, deriv.second);
            }
            global_t_.setFromSortedTriplets(triplets_.begin(), triplets_.end());
        }

        auto fit_local_pars() -> VoidError
        {
            // NOTE: Multiplications will trigger temporary object (memory allocation later during the assignment.)
            Eigen::internal::set_is_malloc_allowed(false);
            buffers_.local_weighted_t.noalias() = local_t_ * sigmas_.asDiagonal();

            buffers_.local_weighted_square.noalias() = buffers_.local_weighted_t.lazyProduct(local_t_.transpose());
            // TODO: What if the inversion fails?
            buffers_.cholesky_solver.compute(buffers_.local_weighted_square);
            buffers_.local_weighted_square_inv.setIdentity();
            buffers_.cholesky_solver.solveInPlace(buffers_.local_weighted_square_inv);
            if (buffers_.cholesky_solver.info() != Eigen::ComputationInfo::Success)
            {
                return std::unexpected{ ErrorCode::analysis_local_fit_rank_deficit };
            }

            buffers_.local_solutions.noalias() =
                buffers_.local_weighted_square_inv.lazyProduct(buffers_.local_weighted_t).lazyProduct(measurements_);

            Eigen::internal::set_is_malloc_allowed(true);

            return {};
        }

        auto calculate_local_fit_chi_square() -> EnumError<std::pair<std::size_t, double>>
        {
            Eigen::internal::set_is_malloc_allowed(false);
            const auto entrypoint_size = Base<DataType>::get_current_state().n_points;
            const auto local_size = buffers_.local_solutions.rows();
            const auto ndf = entrypoint_size - local_size;

            if (ndf < 1)
            {
                return std::unexpected{ ErrorCode::analysis_local_fit_low_stat };
            }

            buffers_.residual_values.noalias() = measurements_ - (local_t_.transpose() * buffers_.local_solutions);
            const auto chi_square = buffers_.residual_values.dot(sigmas_.asDiagonal() * buffers_.residual_values);
            Eigen::internal::set_is_malloc_allowed(true);
            return std::pair{ ndf, chi_square };
        }

        auto update_global_factor_matrix() -> VoidError
        {
            // TODO: Perform the production using index accessing.
            // NOTE: Seems that there is no way to prevent memory allocation with sparse matrices.
            // Eigen::internal::set_is_malloc_allowed(false);
            buffers_.sigmas_sparse_view = sigmas_.asDiagonal();
            buffers_.global_local_weighted_t = local_t_.sparseView();
            buffers_.global_local_weighted_t =
                buffers_.global_local_weighted_t * buffers_.sigmas_sparse_view * global_t_.transpose();
            buffers_.global_weighted_square = global_t_ * buffers_.sigmas_sparse_view * global_t_.transpose();
            buffers_.local_weighted_square_inv_sparse = buffers_.local_weighted_square_inv.sparseView().eval();

            buffers_.global_square_update = buffers_.global_local_weighted_t;
            buffers_.global_square_update = buffers_.global_square_update.transpose() *
                                            buffers_.local_weighted_square_inv_sparse *
                                            buffers_.global_local_weighted_t;
            buffers_.global_square_update += buffers_.global_weighted_square;
            assert(buffers_.global_square_update.isApprox(buffers_.global_square_update.transpose()));
            globals_.factor_matrix += buffers_.global_square_update;
            // Eigen::internal::set_is_malloc_allowed(true);
            return {};
        }

        auto update_global_rhs_vector() -> VoidError
        {
            // TODO: Perform the production using index accessing.
            // NOTE: Seems that there is no way to prevent memory allocation with sparse matrices.
            buffers_.global_rhs_vector_update = (sigmas_.asDiagonal() * measurements_).sparseView();
            buffers_.global_rhs_vector_update = global_t_ * buffers_.global_rhs_vector_update;
            globals_.rhs_vec += buffers_.global_rhs_vector_update;
            buffers_.global_rhs_vector_update = buffers_.local_solutions.sparseView();
            buffers_.global_rhs_vector_update =
                buffers_.global_local_weighted_t.transpose() * buffers_.global_rhs_vector_update;
            globals_.rhs_vec -= buffers_.global_rhs_vector_update;

            return {};
        }

        static void resize_globals(Globals& globals, std::size_t n_globals)
        {
            const auto num_of_globals = static_cast<long>(n_globals);
            globals.rhs_vec.resize(num_of_globals);
            globals.rhs_vec.setZero();
            globals.factor_matrix.resize(num_of_globals, num_of_globals);
            globals.factor_matrix.setZero();
        }

        static auto find_redundant_parameter_idx(const auto& eigen_solver, Result<DataType>& result)
        {
            result.redundant_parameter_indices.clear();
            const auto& unitary_matrix = eigen_solver.eigenvectors();
            const auto& eigen_values = eigen_solver.eigenvalues();
            const auto n_globals = eigen_values.rows();
            auto prob_mat = Eigen::MatrixX<DataType>::Zero(n_globals, n_globals).eval();

            for (const auto [idx, val] : std::views::zip(std::views::iota(0), eigen_values))
            {
                if (std::abs(val) < Eigen::NumTraits<DataType>::dummy_precision())
                {
                    prob_mat(idx, idx) = 1;
                }
            }
            const auto diagonal_values = (unitary_matrix * prob_mat * unitary_matrix.transpose()).diagonal().eval();
            for (const auto [idx, diagonal_val] : std::views::zip(std::views::iota(0), diagonal_values))
            {
                if (std::abs(diagonal_val) > Eigen::NumTraits<DataType>::dummy_precision())
                {
                    result.redundant_parameter_indices.push_back(idx);
                }
            }
        }

        static void check_rank_deficit(const Globals& globals, Result<DataType>& result)
        {
            result.eigen_values.clear();
            auto eigen_solver = Eigen::SelfAdjointEigenSolver<typename Globals::MatrixType>{ globals.factor_matrix };
            const auto& eigen_values = eigen_solver.eigenvalues();
            std::ranges::copy(eigen_solver.eigenvalues(), std::back_inserter(result.eigen_values));
            result.rank_deficit = 0;
            for (const auto [idx, val] : std::views::zip(std::views::iota(0), eigen_values))
            {
                if (val + std::numeric_limits<DataType>::epsilon() < 0)
                {
                    result.error_status = ErrorCode::analysis_global_negative_definite;
                    return;
                }
                if (std::abs(val) < std::numeric_limits<DataType>::epsilon())
                {
                    ++result.rank_deficit;
                }
            }

            if (result.rank_deficit != 0)
            {
                result.error_status = ErrorCode::analysis_rank_deficit;
                find_redundant_parameter_idx(eigen_solver, result);
                return;
            }
        }
    };
}; // namespace centipede::core::engine
