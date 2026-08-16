#include "centipede/util/common_definitions.hpp"
#include "centipede/util/error_types.hpp"
#include <cassert>
#include <concepts>
#include <cstddef>
#include <indicators/color.hpp>
#include <indicators/font_style.hpp>
#include <indicators/progress_bar.hpp>
#include <indicators/setting.hpp>
#include <ranges>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace centipede
{
    // TODO:
    //  Error Handling!
    //  Ehm, testing?

    template <typename F>
    concept is_increment_function = std::invocable<F> && std::same_as<std::invoke_result_t<F>, std::size_t>;

    class ProgressIndicator
    {
      public:
        struct Config
        {
            bool enable_percentage = true;
            std::size_t bar_width = common::DEFAULT_INDICATOR_BAR_WIDTH;
            std::string label_text;
        };

        ProgressIndicator() = default;

        explicit ProgressIndicator(Config config)
            : config_(std::move(config))
        {
        }

        ProgressIndicator(ProgressIndicator&&) = delete;
        ProgressIndicator& operator=(ProgressIndicator&&) = delete;
        ProgressIndicator(const ProgressIndicator&) = delete;
        ProgressIndicator& operator=(const ProgressIndicator&) = delete;

        auto get_adaptor() & { return adaptor_; }

        template <typename... Args>
        auto get_adaptor(Args&&... args) &
        {
            return adaptor_(std::forward<Args>(args)...);
        }

        template <typename... Args>
        auto get_adaptor(Args&&...) && = delete;

      private:
        template <std::ranges::view BaseView, is_increment_function IncrementFunctionT>
        struct ProgressView : std::ranges::view_interface<ProgressView<BaseView, IncrementFunctionT>>
        {
            using IteratorType = std::ranges::iterator_t<BaseView>;
            using SentinelType = std::ranges::sentinel_t<BaseView>;

            ProgressView(BaseView&& view,
                         std::size_t total_size,
                         IncrementFunctionT&& inc_func,
                         ProgressIndicator* indicator)
                : base_view(std::move(view))
                , total_size_n(total_size)
                , increment_function(std::move(inc_func))
                , progress_indicator(indicator)
            {
                assert(progress_indicator);
            }

            auto begin()
            {
                assert(progress_indicator);

                if (total_size_n == 0UZ)
                {
                    progress_indicator->status_ = ErrorCode::progress_zero_size;
                    progress_indicator->bar_.mark_as_completed();
                }

                return Iterator{ this, std::ranges::begin(base_view), std::ranges::end(base_view) };
            }

            auto end() { return Sentinel{}; }

            struct Sentinel
            {
            };

            class Iterator
            {
              public:
                Iterator(ProgressView* progress_view, IteratorType current_it, SentinelType end_it)
                    : progress_view_(progress_view)
                    , current_it_(current_it)
                    , end_it_(end_it)
                {
                    assert(progress_view_);
                    assert(progress_view_->progress_indicator);
                }

                auto operator++() -> Iterator&
                {
                    assert(current_it_ != end_it_);

                    add_progress();
                    ++current_it_;

                    return *this;
                }

                decltype(auto) operator*() const
                {
                    assert(current_it_ != end_it_);
                    return *current_it_;
                }

                bool operator==(Sentinel) const { return current_it_ == end_it_; }

                bool operator!=(Sentinel sentinel) const { return !(*this == sentinel); }

              private:
                void add_progress()
                {
                    assert(progress_view_);
                    assert(progress_view_->progress_indicator);

                    auto& indicator = *progress_view_->progress_indicator;

                    if (progress_view_->total_size_n == 0UZ)
                    {
                        indicator.status_ = ErrorCode::progress_zero_size;
                        return;
                    }

                    assert(count_n_ <= progress_view_->total_size_n);

                    const auto increment = progress_view_->increment_function(); // NOTE: std::invoke?

                    if (increment == 0UZ)
                    {
                        indicator.status_ = ErrorCode::progress_inc_returns_zero;
                        return;
                    }

                    const auto remaining = progress_view_->total_size_n - count_n_;

                    if (increment > remaining)
                    {
                        count_n_ = progress_view_->total_size_n;

                        indicator.status_ = ErrorCode::progress_inc_exceeds_size;
                    }
                    else
                    {
                        count_n_ += increment;
                    }

                    const auto percent = 100UZ * count_n_ / progress_view_->total_size_n;

                    indicator.bar_.set_progress(percent);

                    if (count_n_ == progress_view_->total_size_n)
                    {
                        indicator.status_ = ErrorCode::success;
                    }
                }

                ProgressView* progress_view_ = nullptr;
                IteratorType current_it_;
                SentinelType end_it_;
                std::size_t count_n_{};
            };

            BaseView base_view;
            std::size_t total_size_n{};
            IncrementFunctionT increment_function;
            ProgressIndicator* progress_indicator = nullptr;
        };

        struct ProgressAdaptor : std::ranges::range_adaptor_closure<ProgressAdaptor>
        {
            ProgressAdaptor(ProgressIndicator* indicator)
                : progress_indicator(indicator)
            {
            }

            auto operator()(std::ranges::sized_range auto&& range)
            {
                return ProgressView{ std::views::all(std::forward<decltype(range)>(range)),
                                     std::ranges::size(range),
                                     []() { return 1UZ; },
                                     progress_indicator };
            }

            auto operator()(std::ranges::viewable_range auto&& range,
                            std::size_t total_size,
                            is_increment_function auto&& inc_func)
            {
                return ProgressView{ std::views::all(std::forward<decltype(range)>(range)),
                                     total_size,
                                     std::move(inc_func),
                                     progress_indicator };
            }

            auto operator()(std::ranges::viewable_range auto&& range, std::size_t total_size)
            {
                return ProgressView{ std::views::all(std::forward<decltype(range)>(range)),
                                     total_size,
                                     []() { return 1UZ; },
                                     progress_indicator };
            }

            auto operator()(std::size_t total_size)
            {
                return ProgressClosure{ total_size, []() { return 1UZ; }, progress_indicator };
            }

            auto operator()(std::size_t total_size, is_increment_function auto&& inc_func)
            {
                return ProgressClosure{ total_size, std::move(inc_func), progress_indicator };
            }

            template <is_increment_function IncrementFunctionT>
            struct ProgressClosure : std::ranges::range_adaptor_closure<ProgressClosure<IncrementFunctionT>>
            {
                ProgressClosure(std::size_t total_size, IncrementFunctionT&& inc_func, ProgressIndicator* indicator)
                    : total_size_n(total_size)
                    , increment_function(std::move(inc_func))
                    , progress_indicator(indicator)
                {
                }

                auto operator()(std::ranges::viewable_range auto&& range)
                {
                    return ProgressView{ std::views::all(std::forward<decltype(range)>(range)),
                                         total_size_n,
                                         std::move(increment_function),
                                         progress_indicator };
                }

                std::size_t total_size_n{};
                IncrementFunctionT increment_function;
                ProgressIndicator* progress_indicator = nullptr;
            };

            ProgressIndicator* progress_indicator = nullptr;
        };

        Config config_{};

        indicators::ProgressBar bar_{ indicators::option::BarWidth{ config_.bar_width },
                                      indicators::option::Start{ " [" },
                                      indicators::option::Fill{ "=" },
                                      indicators::option::Lead{ ">" },
                                      indicators::option::Remainder{ "-" },
                                      indicators::option::End{ "]" },
                                      indicators::option::PrefixText{ config_.label_text },
                                      indicators::option::ForegroundColor{ indicators::Color::yellow },
                                      indicators::option::ShowPercentage{ true },
                                      indicators::option::ShowElapsedTime{ true },
                                      indicators::option::ShowRemainingTime{ true },
                                      indicators::option::FontStyles{
                                          std::vector<indicators::FontStyle>{ indicators::FontStyle::bold } } };
        ErrorCode status_{};
        ProgressAdaptor adaptor_{ this };
    };
} // namespace centipede

/********************* OLD *************************/

// using ProgressFontStyle = indicators::FontStyle;
// using ProgressColor = indicators::Color;
//
// class ProgressAdaptor : public std::ranges::range_adaptor_closure<ProgressAdaptor>
// {
//   public:
//     using IncrementFunT = std::function<std::size_t()>;
//
//     ProgressAdaptor() = default;
//
//     template <typename... iopts>
//     explicit ProgressAdaptor(iopts&&... options)
//         : bar_ptr_{ std::make_shared<indicators::ProgressBar>(std::forward<iopts>(options)...) }
//     {
//     }
//
//     template <typename RangeT>
//         requires std::ranges::range<RangeT>
//     using BaseView = std::views::all_t<RangeT>;
//
//     struct ProgressClosure : std::ranges::range_adaptor_closure<ProgressClosure>
//     {
//         std::size_t total_size_n;
//         IncrementFunT increment_fun;
//
//         std::shared_ptr<indicators::ProgressBar> bar_ptr;
//         std::shared_ptr<ErrorCode> status_ptr;
//
//         template <typename RangeT>
//             requires std::ranges::range<RangeT>
//         auto operator()(RangeT&& range)
//         {
//             using ViewT = std::views::all_t<RangeT>;
//
//             return ProgressView<ViewT>{
//                 std::views::all(std::forward<RangeT>(range)), total_size_n, increment_fun, bar_ptr,
//                 status_ptr
//             };
//         }
//     };
//
//     template <typename RangeT>
//         requires std::ranges::range<RangeT>
//     auto operator()(RangeT&& range, std::size_t total_size_n, IncrementFunT increment_fun)
//     {
//         return ProgressView<BaseView<RangeT>>{ std::views::all(std::forward<RangeT>(range)),
//                                                total_size_n,
//                                                std::move(increment_fun),
//                                                bar_ptr_,
//                                                status_ptr_ };
//     }
//
//     template <typename RangeT>
//         requires std::ranges::range<RangeT>
//     auto operator()(RangeT&& range, std::size_t total_size_n)
//     {
//         return ProgressView<BaseView<RangeT>>{ std::views::all(std::forward<RangeT>(range)),
//                                                total_size_n,
//                                                []() -> std::size_t { return 1UZ; },
//                                                bar_ptr_,
//                                                status_ptr_ };
//     }
//
//     template <typename RangeT>
//         requires std::ranges::sized_range<RangeT>
//     auto operator()(RangeT&& range)
//     {
//         return ProgressView<BaseView<RangeT>>{ std::views::all(std::forward<RangeT>(range)),
//                                                std::ranges::size(range),
//                                                []() -> std::size_t { return 1UZ; },
//                                                bar_ptr_,
//                                                status_ptr_ };
//     }
//
//     auto operator()(std::size_t total_size_n)
//     {
//         return ProgressClosure{ {}, total_size_n, []() -> std::size_t { return 1UZ; }, bar_ptr_, status_ptr_
//         };
//     }
//
//     auto operator()(std::size_t total_size_n, IncrementFunT increment_fun)
//     {
//         return ProgressClosure{ {}, total_size_n, std::move(increment_fun), bar_ptr_, status_ptr_ };
//     }
//
//     [[nodiscard]] auto get_status() const -> ErrorCode { return *status_ptr_; }
//
//     template <typename RangeT>
//         requires std::ranges::range<RangeT>
//     struct ProgressView
//     {
//         using BaseView = std::views::all_t<RangeT>;
//         using IteratorType = std::ranges::iterator_t<RangeT>;
//         using SentinelType = std::ranges::sentinel_t<RangeT>;
//
//         ProgressView(BaseView base_view,
//                      std::size_t total_size_n,
//                      IncrementFunT increment_fun,
//                      std::shared_ptr<indicators::ProgressBar> bar_ptr,
//                      std::shared_ptr<ErrorCode> status_ptr)
//             : base_view_(std::move(base_view))
//             , total_size_n_(total_size_n)
//             , increment_fun_(std::move(increment_fun))
//             , bar_ptr_(std::move(bar_ptr))
//             , status_ptr_(std::move(status_ptr))
//         {
//             assert(bar_ptr_);
//             assert(status_ptr_);
//             assert(increment_fun_);
//         }
//
//         auto get_status() -> ErrorCode { return *status_ptr_; }
//
//         auto begin()
//         {
//             if (total_size_n_ == 0UZ)
//             {
//                 *status_ptr_ = ErrorCode::progress_zero_size;
//
//                 bar_ptr_->mark_as_completed();
//             }
//
//             return Iterator{ this, std::ranges::begin(base_view_), std::ranges::end(base_view_) };
//         }
//
//         auto end() { return Sentinel{}; }
//
//         struct Sentinel
//         {
//         };
//
//         class Iterator
//         {
//           public:
//             Iterator(ProgressView* progress_view, IteratorType current_it, SentinelType end_it)
//                 : progress_view_(progress_view)
//                 , current_it_(current_it)
//                 , end_it_(end_it)
//             {
//                 assert(progress_view_);
//             }
//
//             auto operator++() -> Iterator&
//             {
//                 assert(current_it_ != end_it_);
//                 add_progress();
//                 ++current_it_;
//                 return *this;
//             }
//
//             auto operator*()
//             {
//                 assert(current_it_ != end_it_);
//                 return *current_it_;
//             }
//
//             bool operator==(Sentinel) { return current_it_ == end_it_; }
//
//             bool operator!=(Sentinel sentinel) { return !(*this == sentinel); }
//
//             bool operator==(Sentinel) const { return current_it_ == end_it_; }
//
//             bool operator!=(Sentinel sentinel) const { return !(*this == sentinel); }
//
//             void add_progress()
//             {
//                 assert(progress_view_);
//                 assert(progress_view_->bar_ptr_);
//                 assert(progress_view_->status_ptr_);
//                 assert(progress_view_->increment_fun_);
//                 assert(count_n_ <= progress_view_->total_size_n_);
//                 const auto increment = progress_view_->increment_fun_();
//
//                 if (increment == 0UZ)
//                 {
//                     *progress_view_->status_ptr_ = ErrorCode::progress_inc_returns_zero;
//                     return;
//                 }
//
//                 const auto remaining = progress_view_->total_size_n_ - count_n_;
//
//                 if (increment > remaining)
//                 {
//                     count_n_ = progress_view_->total_size_n_;
//
//                     *progress_view_->status_ptr_ = ErrorCode::progress_inc_exceeds_size;
//                 }
//                 else
//                 {
//                     count_n_ += increment;
//                 }
//
//                 const auto percent = 100UZ * count_n_ / progress_view_->total_size_n_;
//
//                 progress_view_->bar_ptr_->set_progress(percent);
//
//                 if (count_n_ == progress_view_->total_size_n_)
//                 {
//                     *(progress_view_->status_ptr_) = ErrorCode::success;
//                 }
//             }
//
//           private:
//             ProgressView* progress_view_;
//             IteratorType current_it_;
//             SentinelType end_it_;
//             std::size_t count_n_{};
//         };
//
//       private:
//         BaseView base_view_;
//         std::size_t total_size_n_;
//         IncrementFunT increment_fun_;
//         std::shared_ptr<indicators::ProgressBar> bar_ptr_ = nullptr;
//         std::shared_ptr<ErrorCode> status_ptr_ = nullptr;
//     };
//
//   private:
//     std::shared_ptr<indicators::ProgressBar> bar_ptr_{ std::make_shared<indicators::ProgressBar>() };
//     std::shared_ptr<ErrorCode> status_ptr_ = std::make_shared<ErrorCode>(ErrorCode::incomplete);
// };

// } // namespace centipede
