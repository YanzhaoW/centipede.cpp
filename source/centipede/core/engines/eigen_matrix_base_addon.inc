template <typename Index>
EIGEN_DEVICE_FUNC EIGEN_ALWAYS_INLINE auto operator[](Index row, Index col) -> Scalar& {
    return this->operator()(row, col);
}

// EIGEN_DEVICE_FUNC EIGEN_ALWAYS_INLINE auto operator[](Index idx) -> Scalar& {
//     return this->operator()(idx);
// }
