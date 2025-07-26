// Support for std::format is really spotty.
// Gcc12 does not support it.
// Eventually replace with std::format when that is widely available.
#include <fmt/format.h>
#include <fmt/ranges.h>

namespace fmt {
template <>
struct formatter<pmtv::map_t::value_type> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const pmtv::map_t::value_type& kv, FormatContext& ctx) const {
        return fmt::format_to(ctx.out(), "{}: {}", kv.first, kv.second);
    }
};

template <pmtv::Complex C>
struct formatter<C> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const C& arg, FormatContext& ctx) const {
        if (arg.imag() >= 0)
            return fmt::format_to(ctx.out(), "{0}+j{1}", arg.real(), arg.imag());
        else
            return fmt::format_to(ctx.out(), "{0}-j{1}", arg.real(), -arg.imag());
    }
};

template<pmtv::IsPmt P>
struct formatter<P>
{

    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const P& value, FormatContext& ctx) const {
        return std::visit([&ctx](const auto arg) {
            using namespace pmtv;
            using T = std::decay_t<decltype(arg)>;
            if constexpr (Scalar<T> || Complex<T>)
                return fmt::format_to(ctx.out(), "{}", arg);
            else if constexpr (std::same_as<T, std::string>)
                return fmt::format_to(ctx.out(), "{}",  arg);
            else if constexpr (PmtTensor<T>) {
                // Difficult to format an N-dim Tensor.  Figure out something eventually.
                return fmt::format_to(ctx.out(), "[{}]", fmt::join(arg.data_span(), ", "));
            } else if constexpr (UniformStringVector<T>)
                return fmt::format_to(ctx.out(), "[{}]", fmt::join(arg, ", "));
            else if constexpr (std::same_as<T, std::vector<pmt>>) {
                return fmt::format_to(ctx.out(), "[{}]", fmt::join(arg, ", "));
            } else if constexpr (PmtMap<T>) {
                return fmt::format_to(ctx.out(), "{{{}}}", fmt::join(arg, ", "));
            } else if constexpr (std::same_as<std::monostate, T>)
                return fmt::format_to(ctx.out(), "null");
            return fmt::format_to(ctx.out(), "unknown type {}", typeid(T).name());
            }, value);

    }
};

} // namespace fmt

namespace pmtv {
    template <IsPmt P>
    std::ostream& operator<<(std::ostream& os, const P& value) {
        os << fmt::format("{}", value);
        return os;
    }
}
