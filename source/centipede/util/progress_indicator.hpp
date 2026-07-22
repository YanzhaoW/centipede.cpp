#include "centipede/util/error_types.hpp"
#include <cstddef>
#include <functional>
#include <indicators/color.hpp>
#include <indicators/font_style.hpp>
#include <indicators/progress_bar.hpp>
#include <indicators/setting.hpp>
#include <memory>
#include <ranges>
#include <utility>

namespace centipede::progress
{
    // TODO:
    //  Error Handling!
    //  Ehm, testing?

    namespace config = indicators::option;
    using ProgressFontStyle = indicators::FontStyle;
    using ProgressColor = indicators::Color;

    class ProgressAdaptor : public std::ranges::range_adaptor_closure<ProgressAdaptor>
    {
      public:
        using IncrementFunT = std::function<std::size_t()>;

        ProgressAdaptor() = default;

        template <typename... Options>
        explicit ProgressAdaptor(Options&&... options)
            : bar_ptr_{ std::make_shared<indicators::ProgressBar>(std::forward<Options>(options)...) }
        {
        }

        template <typename RangeT>
            requires std::ranges::range<RangeT>
        using BaseView = std::views::all_t<RangeT>;
        struct ProgressClosure : std::ranges::range_adaptor_closure<ProgressClosure>
        {
            ProgressAdaptor* adaptor;
            std::size_t total_size_n;
            IncrementFunT increment_fun;

            template <typename RangeT>
                requires std::ranges::range<RangeT>
            auto operator()(RangeT&& range)
            {
                return (*adaptor)(std::forward<RangeT>(range), total_size_n, increment_fun);
            }
        };
        template <typename RangeT>
            requires std::ranges::range<RangeT>
        auto operator()(RangeT&& range, std::size_t total_size_n, IncrementFunT increment_fun)
        {
            return ProgressView<BaseView<RangeT>>{ std::views::all(std::forward<RangeT>(range)),
                                                   total_size_n,
                                                   std::move(increment_fun),
                                                   bar_ptr_,
                                                   status_ptr_ };
        }

        template <typename RangeT>
            requires std::ranges::range<RangeT>
        auto operator()(RangeT&& range, std::size_t total_size_n)
        {
            return ProgressView<BaseView<RangeT>>{ std::views::all(std::forward<RangeT>(range)),
                                                   total_size_n,
                                                   []() -> std::size_t { return 1UZ; },
                                                   bar_ptr_,
                                                   status_ptr_ };
        }

        template <typename RangeT>
            requires std::ranges::sized_range<RangeT>
        auto operator()(RangeT&& range)
        {
            return ProgressView<BaseView<RangeT>>{ std::views::all(std::forward<RangeT>(range)),
                                                   std::ranges::size(range),
                                                   []() -> std::size_t { return 1UZ; },
                                                   bar_ptr_,
                                                   status_ptr_ };
        }

        auto operator()(std::size_t total_size_n)
        {
            return ProgressClosure{ {}, this, total_size_n, []() -> std::size_t { return 1UZ; } };
        }

        auto operator()(std::size_t total_size_n, IncrementFunT increment_fun)
        {
            return ProgressClosure{ {}, this, total_size_n, std::move(increment_fun) };
        }

        [[nodiscard]] auto get_status() const -> ErrorCode { return *status_ptr_; }

        template <typename RangeT>
            requires std::ranges::range<RangeT>
        struct ProgressView
        {
            using BaseView = std::views::all_t<RangeT>;
            using IteratorType = std::ranges::iterator_t<RangeT>;
            using SentinelType = std::ranges::sentinel_t<RangeT>;

            ProgressView(BaseView base_view,
                         std::size_t total_size_n,
                         IncrementFunT increment_fun,
                         std::shared_ptr<indicators::ProgressBar> bar_ptr,
                         std::shared_ptr<ErrorCode> status_ptr)
                : base_view_(std::move(base_view))
                , total_size_n_(total_size_n)
                , increment_fun_(std::move(increment_fun))
                , bar_ptr_(std::move(bar_ptr))
                , status_ptr_(std::move(status_ptr))
            {
            }

            auto get_status() -> ErrorCode { return *status_ptr_; }

            auto begin()
            {
                if (total_size_n_ == 0UZ)
                {
                    *status_ptr_ = ErrorCode::progress_zero_size;

                    bar_ptr_->mark_as_completed();
                }

                return Iterator{ this, std::ranges::begin(base_view_), std::ranges::end(base_view_) };
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
                }

                auto operator++()
                {
                    add_progress();
                    ++current_it_;
                    ++element_count_;
                    return *this;
                }

                auto operator*() { return *current_it_; }

                bool operator==(Sentinel)
                {
                    return current_it_ == end_it_ or element_count_ >= progress_view_->total_size_n_;
                }

                bool operator!=(Sentinel sentinel) { return !(*this == sentinel); }

                bool operator==(Sentinel) const
                {
                    return current_it_ == end_it_ or element_count_ >= progress_view_->total_size_n_;
                }

                bool operator!=(Sentinel sentinel) const { return !(*this == sentinel); }

                void add_progress()
                {
                    const auto increment = progress_view_->increment_fun_();

                    if (increment == 0UZ)
                    {
                        *progress_view_->status_ptr_ = ErrorCode::progress_inc_returns_zero;
                        return;
                    }

                    const auto remaining = progress_view_->total_size_n_ - count_n_;

                    if (increment > remaining)
                    {
                        count_n_ = progress_view_->total_size_n_;

                        *progress_view_->status_ptr_ = ErrorCode::progress_inc_exceeds_size;
                    }
                    else
                    {
                        count_n_ += increment;

                        *progress_view_->status_ptr_ = ErrorCode::success;
                    }

                    const auto percent = 100UZ * count_n_ / progress_view_->total_size_n_;

                    progress_view_->bar_ptr_->set_progress(percent);
                }

              private:
                ProgressView* progress_view_;
                IteratorType current_it_;
                SentinelType end_it_;
                std::size_t element_count_{};
                std::size_t count_n_{};
            };

          private:
            BaseView base_view_;
            std::size_t total_size_n_;
            IncrementFunT increment_fun_;
            std::shared_ptr<indicators::ProgressBar> bar_ptr_ = nullptr;
            std::shared_ptr<ErrorCode> status_ptr_ = nullptr;
        };

      private:
        std::shared_ptr<indicators::ProgressBar> bar_ptr_{ std::make_shared<indicators::ProgressBar>() };
        std::shared_ptr<ErrorCode> status_ptr_ = std::make_shared<ErrorCode>(ErrorCode::incomplete);
    };
} // namespace centipede::progress
