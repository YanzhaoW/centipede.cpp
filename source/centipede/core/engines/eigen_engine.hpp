#pragma once

#include "centipede/core/engines/base_engine.hpp"
#include "centipede/data/entry.hpp"
#include <Eigen/Core>
#include <Eigen/SparseCore>
#include <vector>

namespace centipede::core::engines
{
    template <typename DataType>
    class Engine<MatrixEngineType::eigen, DataType> : public Base<DataType>
    {
      private:
        std::vector<Eigen::Triplet<DataType>> triplets_;
        Eigen::SparseMatrix<DataType> global_derivs_{};
        Eigen::MatrixX<DataType> local_derivs_{};

        friend Base<DataType>;

        void fill_local_derivs(const Entry<DataType>::Deriv& data)
        {
            const auto entrypoint_size = get_current_entrypoint_size();
        }

        void fill_global_derivs(const Entry<DataType>::Deriv& data) {}

        void fit_local_pars() {}
    };
}; // namespace centipede::core::engines
