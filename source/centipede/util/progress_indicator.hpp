#include <cstddef>
#include <indicators/progress_bar.hpp>
#include <ranges>

using namespace indicators;
class ProgressAdaptor;

template <typename IncrementFunT>
struct ProgressClosure : std::ranges::range_adaptor_closure<ProgressClosure<IncrementFunT>>
{
    ProgressAdaptor* adaptor;
    std::size_t total_size_n;
    IncrementFunT increment_fun;

    template <typename RangeT>
    auto operator()(RangeT range)
    {
        return (*adaptor)(std::forward<RangeT>(range), total_size_n, increment_fun);
    }
};

class ProgressAdaptor
{
  public:
    template <typename RangeT, typename IncrementFunT>
    auto operator()(RangeT&& range, std::size_t total_size_n, IncrementFunT increment_fun)
    {
        return ProgressView<RangeT, IncrementFunT>{ this, std::forward<RangeT>(range), total_size_n, increment_fun };
    }

    template <typename IncrementFunT>
    auto operator()(std::size_t total_size_n, IncrementFunT increment_fun)
    {
        return ProgressClosure<IncrementFunT>{ {}, this, total_size_n, increment_fun };
    }

    template <typename RangeT, typename IncrementFunT>
    struct ProgressView
    {
        using IteratorType = std::ranges::iterator_t<RangeT>;

        ProgressView(ProgressAdaptor* progress_adaptor,
                     RangeT&& base_range,
                     std::size_t total_size_n,
                     IncrementFunT increment_fun)
            : progress_adaptor_(progress_adaptor)
            , base_range_(std::move(base_range))
            , total_size_n_(total_size_n)
            , increment_fun_(increment_fun)
        {
        }

        auto begin() { return Iterator{ progress_adaptor_, this, base_range_.begin() }; }

        auto end() { return base_range_.end(); }

        class Iterator
        {
          public:
            Iterator(ProgressAdaptor* progress_adaptor, ProgressView* progress_view, IteratorType current_it)
                : progress_adaptor_(progress_adaptor)
                , progress_view_(progress_view)
                , current_it_(current_it)
            {
            }

            auto operator++()
            {
                ++current_it_;
                count_n_ += progress_view_->increment_fun_();
                progress_adaptor_->bar_.set_progress(100 * count_n_ / progress_view_->total_size_n_);
            }

            auto operator*() { return *current_it_; }

            auto operator==(auto& other) { return current_it_ == other; }
            auto operator!=(auto& other) { return current_it_ != other; }
            auto operator==(const auto& other) const { return current_it_ == other; }
            auto operator!=(const auto& other) const { return current_it_ != other; }

          private:
            ProgressAdaptor* progress_adaptor_;
            ProgressView* progress_view_;
            IteratorType current_it_;
            std::size_t count_n_{};
        };

      private:
        RangeT base_range_;
        std::size_t total_size_n_;
        IncrementFunT increment_fun_;
        ProgressAdaptor* progress_adaptor_;
    };

  private:
    ProgressBar bar_{ option::BarWidth{ 50 },
                      option::Start{ "[" },
                      option::Fill{ "=" },
                      option::Lead{ ">" },
                      option::Remainder{ " " },
                      option::End{ "]" },
                      option::PostfixText{ "Extracting Archive" },
                      option::ForegroundColor{ Color::green },
                      option::ShowPercentage{ true },
                      option::FontStyles{ std::vector<FontStyle>{ FontStyle::bold } } };
};
