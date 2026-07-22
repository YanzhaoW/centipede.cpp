#include "centipede/util/error_types.hpp"
#include <cstddef>
#include <functional>
#include <indicators/color.hpp>
#include <indicators/font_style.hpp>
#include <indicators/progress_bar.hpp>
#include <indicators/setting.hpp>
#include <ranges>
#include <utility>
#include <vector>

namespace centipede::progress
{
    // TODO:
    //  Overload ProgressAdaptor () to:
    //  1) iterate over n elements (instead of increment_fun)
    //  2) iterate by 1 (for standard ranges)
    //  ProgressAdaptor constructor that takes some config struct (to config the prog bar)
    //  Error Handling!
    //  Ehm, testing?

    namespace config = indicators::option;
    using ProgressFontStyle = indicators::FontStyle;
    using ProgressColor = indicators::Color;

    class ProgressAdaptor
    {
      public:
        using IncrementFunT = std::function<std::size_t()>;

        ProgressAdaptor() = default;

        template <typename... Options>
        explicit ProgressAdaptor(Options&&... options)
            : bar_{ std::forward<Options>(options)... }
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
            return ProgressView<BaseView<RangeT>>{
                this, std::views::all(std::forward<RangeT>(range)), total_size_n, std::move(increment_fun)
            };
        }

        template <typename RangeT>
            requires std::ranges::range<RangeT>
        auto operator()(RangeT&& range, std::size_t total_size_n)
        {
            return ProgressView<BaseView<RangeT>>{ this,
                                                   std::views::all(std::forward<RangeT>(range)),
                                                   total_size_n,
                                                   std::move([]() -> std::size_t { return 1UZ; }) };
        }

        auto operator()(std::size_t total_size_n)
        {
            return ProgressClosure{ {}, this, total_size_n, []() -> std::size_t { return 1UZ; } };
        }

        auto operator()(std::size_t total_size_n, IncrementFunT increment_fun)
        {
            return ProgressClosure{ {}, this, total_size_n, std::move(increment_fun) };
        }

        [[nodiscard]] auto get_status() const -> ErrorCode { return status_; }

        template <typename RangeT>
            requires std::ranges::range<RangeT>
        struct ProgressView
        {
            using BaseView = std::views::all_t<RangeT>;
            using IteratorType = std::ranges::iterator_t<RangeT>;
            using SentinelType = std::ranges::sentinel_t<RangeT>;

            ProgressView(ProgressAdaptor* progress_adaptor,
                         BaseView base_view,
                         std::size_t total_size_n,
                         IncrementFunT increment_fun)
                : base_view_(std::move(base_view))
                , total_size_n_(total_size_n)
                , increment_fun_(std::move(increment_fun))
                , progress_adaptor_(progress_adaptor)
            {
            }

            auto begin()
            {
                if (total_size_n_ == 0UZ)
                {
                    progress_adaptor_->status_ = ErrorCode::progress_zero_size;

                    progress_adaptor_->bar_.mark_as_completed();
                }

                return Iterator{
                    progress_adaptor_, this, std::ranges::begin(base_view_), std::ranges::end(base_view_)
                };
            }

            auto end() { return Sentinel{}; }

            struct Sentinel
            {
            };

            class Iterator
            {
              public:
                Iterator(ProgressAdaptor* progress_adaptor,
                         ProgressView* progress_view,
                         IteratorType current_it,
                         SentinelType end_it)
                    : progress_adaptor_(progress_adaptor)
                    , progress_view_(progress_view)
                    , current_it_(current_it)
                    , end_it_(end_it)
                {
                }

                auto operator++()
                {
                    ++current_it_;
                    ++element_count_;
                    add_progress();
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
                        progress_adaptor_->status_ = ErrorCode::progress_inc_returns_zero;
                        return;
                    }

                    if (increment > progress_view_->total_size_n_ - count_n_)
                    {
                        progress_adaptor_->status_ = ErrorCode::progress_inc_exceeds_size;

                        count_n_ = progress_view_->total_size_n_;
                    }
                    else
                    {
                        count_n_ += increment;
                        progress_adaptor_->status_ = ErrorCode::success;
                    }

                    const auto percent = 100UZ * count_n_ / progress_view_->total_size_n_;

                    progress_adaptor_->bar_.set_progress(percent);

                    if (count_n_ >= progress_view_->total_size_n_)
                    {
                        progress_adaptor_->bar_.mark_as_completed();
                        progress_adaptor_->status_ = ErrorCode::success;
                    }
                }

              private:
                ProgressAdaptor* progress_adaptor_;
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
            ProgressAdaptor* progress_adaptor_;
        };

      private:
        indicators::ProgressBar bar_{ config::BarWidth{ 50 },
                                      config::Start{ "[" },
                                      config::Fill{ "=" },
                                      config::Lead{ ">" },
                                      config::Remainder{ " " },
                                      config::End{ "]" },
                                      config::PostfixText{ "Reading binary data" },
                                      config::ForegroundColor{ ProgressColor::green },
                                      config::ShowPercentage{ true },
                                      config::FontStyles{
                                          std::vector<indicators::FontStyle>{ ProgressFontStyle::bold } } };
        ErrorCode status_ = ErrorCode::incomplete;
    };
} // namespace centipede::progress
