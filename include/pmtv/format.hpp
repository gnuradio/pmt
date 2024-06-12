// Support for std::format is really spotty.
// Gcc12 does not support it.
// Eventually replace with std::format when that is widely available.
#include <fmt/format.h>

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
        // Using visit here is fairly slow.  It is probably because of the recursive nature of it.
        // It is really simple to enumerate the possibilities here.
        using namespace pmtv;
        if (std::holds_alternative<std::monostate>(value)) return fmt::format_to(ctx.out(), "null");
        else if (std::holds_alternative<bool>(value)) return fmt::format_to(ctx.out(), "{}", std::get<bool>(value));
        else if (std::holds_alternative<uint8_t>(value)) return fmt::format_to(ctx.out(), "{}", std::get<uint8_t>(value));
        else if (std::holds_alternative<uint16_t>(value)) return fmt::format_to(ctx.out(), "{}", std::get<uint16_t>(value));
        else if (std::holds_alternative<uint32_t>(value)) return fmt::format_to(ctx.out(), "{}", std::get<uint32_t>(value));
        else if (std::holds_alternative<uint64_t>(value)) return fmt::format_to(ctx.out(), "{}", std::get<uint64_t>(value));
        else if (std::holds_alternative<int8_t>(value)) return fmt::format_to(ctx.out(), "{}", std::get<int8_t>(value));
        else if (std::holds_alternative<int16_t>(value)) return fmt::format_to(ctx.out(), "{}", std::get<int16_t>(value));
        else if (std::holds_alternative<int32_t>(value)) return fmt::format_to(ctx.out(), "{}", std::get<int32_t>(value));
        else if (std::holds_alternative<int64_t>(value)) return fmt::format_to(ctx.out(), "{}", std::get<int64_t>(value));
        else if (std::holds_alternative<float>(value)) return fmt::format_to(ctx.out(), "{}", std::get<float>(value));
        else if (std::holds_alternative<double>(value)) return fmt::format_to(ctx.out(), "{}", std::get<double>(value));
        else if (std::holds_alternative<std::complex<float>>(value)) return fmt::format_to(ctx.out(), "{}", std::get<std::complex<float>>(value));
        else if (std::holds_alternative<std::complex<double>>(value)) return fmt::format_to(ctx.out(), "{}", std::get<std::complex<double>>(value));
        else if (std::holds_alternative<std::vector<bool>>(value)) return fmt::format_to(ctx.out(), "[{}]", fmt::join(std::get<std::vector<bool>>(value), ", "));
        else if (std::holds_alternative<std::vector<uint8_t>>(value)) return fmt::format_to(ctx.out(), "[{}]", fmt::join(std::get<std::vector<uint8_t>>(value), ", "));
        else if (std::holds_alternative<std::vector<uint16_t>>(value)) return fmt::format_to(ctx.out(), "[{}]", fmt::join(std::get<std::vector<uint16_t>>(value), ", "));
        else if (std::holds_alternative<std::vector<uint32_t>>(value)) return fmt::format_to(ctx.out(), "[{}]", fmt::join(std::get<std::vector<uint32_t>>(value), ", "));
        else if (std::holds_alternative<std::vector<uint64_t>>(value)) return fmt::format_to(ctx.out(), "[{}]", fmt::join(std::get<std::vector<uint64_t>>(value), ", "));
        else if (std::holds_alternative<std::vector<int8_t>>(value)) return fmt::format_to(ctx.out(), "[{}]", fmt::join(std::get<std::vector<int8_t>>(value), ", "));
        else if (std::holds_alternative<std::vector<int16_t>>(value)) return fmt::format_to(ctx.out(), "[{}]", fmt::join(std::get<std::vector<int16_t>>(value), ", "));
        else if (std::holds_alternative<std::vector<int32_t>>(value)) return fmt::format_to(ctx.out(), "[{}]", fmt::join(std::get<std::vector<int32_t>>(value), ", "));
        else if (std::holds_alternative<std::vector<int64_t>>(value)) return fmt::format_to(ctx.out(), "[{}]", fmt::join(std::get<std::vector<int64_t>>(value), ", "));
        else if (std::holds_alternative<std::vector<float>>(value)) return fmt::format_to(ctx.out(), "[{}]", fmt::join(std::get<std::vector<float>>(value), ", "));
        else if (std::holds_alternative<std::vector<double>>(value)) return fmt::format_to(ctx.out(), "[{}]", fmt::join(std::get<std::vector<double>>(value), ", "));
        else if (std::holds_alternative<std::vector<std::complex<float>>>(value)) return fmt::format_to(ctx.out(), "[{}]", fmt::join(std::get<std::vector<std::complex<float>>>(value), ", "));
        else if (std::holds_alternative<std::vector<std::complex<double>>>(value)) return fmt::format_to(ctx.out(), "[{}]", fmt::join(std::get<std::vector<std::complex<double>>>(value), ", "));
        else if (std::holds_alternative<std::string>(value)) return fmt::format_to(ctx.out(), "{}", std::get<std::string>(value));
        else if (std::holds_alternative<std::vector<std::string>>(value)) return fmt::format_to(ctx.out(), "[{}]", fmt::join(std::get<std::vector<std::string>>(value), ", "));
        else if (std::holds_alternative<std::vector<pmt>>(value)) return fmt::format_to(ctx.out(), "[{}]", fmt::join(std::get<std::vector<pmt>>(value), ", "));
        else if (std::holds_alternative<map_t>(value)) return fmt::format_to(ctx.out(), "{{{}}}", fmt::join(std::get<map_t>(value), ", "));
        //static_assert(false);
        return fmt::format_to(ctx.out(), "error");

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
