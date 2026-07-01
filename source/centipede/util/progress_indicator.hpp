#include <cstddef>
#include <functional>
#include <indicators/color.hpp>
#include <indicators/font_style.hpp>
#include <indicators/progress_bar.hpp>
#include <indicators/setting.hpp>
#include <ranges>
#include <vector>

namespace centipede::progress
{
    // TODO:
    //  Overload ProgressAdaptor () to:
    //  1) iterate over n elements (instead of increment_fun)
    //  2) iterate by 1 (for standard ranges)
    //  ProgressAdaptor constructor that takes some config struct (to config the prog bar)
    //  Error Handling!
    class ProgressAdaptor;

    using IncrementFunT = std::function<std::size_t()>;

    struct ProgressClosure : std::ranges::range_adaptor_closure<ProgressClosure>
    {
        ProgressAdaptor* adaptor;
        std::size_t total_size_n;
        IncrementFunT increment_fun;

        template <typename RangeT>
        auto operator()(RangeT&& range)
        {
            return (*adaptor)(std::forward<RangeT>(range), total_size_n, increment_fun);
        }
    };

    class ProgressAdaptor
    {
      public:
        template <typename RangeT>
            requires std::ranges::range<RangeT>
        auto operator()(RangeT&& range, std::size_t total_size_n, IncrementFunT increment_fun)
        {
            return ProgressView<RangeT>{ this, std::forward<RangeT>(range), total_size_n, increment_fun };
        }

        template <typename RangeT>
            requires std::ranges::range<RangeT>
        auto operator()(RangeT&& range, std::size_t total_size_n)
        {
            return ProgressView<RangeT>{
                this, std::forward<RangeT>(range), total_size_n, []() -> std::size_t { return 1; }
            };
        }

        auto operator()(std::size_t total_size_n)
        {
            return ProgressClosure{ {}, this, total_size_n, []() -> std::size_t { return 1; } };
        }

        auto operator()(std::size_t total_size_n, IncrementFunT increment_fun)
        {
            return ProgressClosure{ {}, this, total_size_n, increment_fun };
        }

        template <typename RangeT>
            requires std::ranges::range<RangeT>
        struct ProgressView
        {
            using IteratorType = std::ranges::iterator_t<RangeT>;
            using SentinelType = std::ranges::sentinel_t<RangeT>;

            ProgressView(ProgressAdaptor* progress_adaptor,
                         RangeT&& base_range,
                         std::size_t total_size_n,
                         IncrementFunT increment_fun)
                : base_range_(base_range)
                , total_size_n_(total_size_n)
                , increment_fun_(increment_fun)
                , progress_adaptor_(progress_adaptor)
            {
            }

            auto begin()
            {
                return Iterator{
                    progress_adaptor_, this, std::ranges::begin(base_range_), std::ranges::end(base_range_)
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
                    return current_it_ == end_it_ || element_count_ >= progress_view_->total_size_n_;
                }

                bool operator!=(Sentinel s) { return !(*this == s); }

                bool operator==(Sentinel) const
                {
                    return current_it_ == end_it_ || element_count_ >= progress_view_->total_size_n_;
                }

                bool operator!=(Sentinel s) const { return !(*this == s); }

                void add_progress()
                {
                    if (finished_)
                    {
                        return;
                    }
                    count_n_ += progress_view_->increment_fun_();
                    const auto percent = 100 * count_n_ / progress_view_->total_size_n_;
                    progress_adaptor_->bar_.set_progress(percent);
                    if (percent == 100)
                    {
                        finished_ = true;
                    }
                }

              private:
                ProgressAdaptor* progress_adaptor_;
                ProgressView* progress_view_;
                IteratorType current_it_;
                SentinelType end_it_;
                std::size_t element_count_{};
                std::size_t count_n_{};
                bool finished_{ false };
            };

          private:
            RangeT base_range_;
            std::size_t total_size_n_;
            IncrementFunT increment_fun_;
            ProgressAdaptor* progress_adaptor_;
        };

      private:
        indicators::ProgressBar bar_{ indicators::option::BarWidth{ 50 },
                                      indicators::option::Start{ "[" },
                                      indicators::option::Fill{ "=" },
                                      indicators::option::Lead{ ">" },
                                      indicators::option::Remainder{ " " },
                                      indicators::option::End{ "]" },
                                      indicators::option::ForegroundColor{ indicators::Color::green },
                                      indicators::option::ShowPercentage{ true },
                                      indicators::option::FontStyles{
                                          std::vector<indicators::FontStyle>{ indicators::FontStyle::bold } } };
    };
} // namespace centipede::progress
