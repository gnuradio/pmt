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
        // Due to an issue with the c++ spec that has since been resolved, we have to do something
        // funky here.  See
        // https://stackoverflow.com/questions/37526366/nested-constexpr-function-calls-before-definition-in-a-constant-expression-con
        // This problem only appears to occur in gcc 11 in certain optimization modes. The problem
        // occurs when we want to format a vector<pmt>.  Ideally, we can write something like:
        //      return fmt::format_to(ctx.out(), "[{}]", fmt::join(arg, ", "));
        // It looks like the issue effects clang 14/15 as well.
        // However, due to the above issue, it fails to compile.  So we have to do the equivalent
        // ourselves.  We can't recursively call the formatter, but we can recursively call a lambda
        // function that does the formatting.
        // It gets more complicated, because we need to pass the function into the lambda.  We can't
        // pass in the lamdba as it is defined, so we create a nested lambda.  Which accepts a function
        // as a argument.
        // Because we are calling std::visit, we can't pass non-variant arguments to the visitor, so we
        // have to create a new nested lambda every time we format a vector to ensure that it works.
        using namespace pmtv;
        using ret_type = decltype(fmt::format_to(ctx.out(), ""));
        auto format_func = [&ctx](const auto format_arg) {
            auto function_main = [&ctx](const auto arg, auto function) -> ret_type {
            using namespace pmtv;
            using T = std::decay_t<decltype(arg)>;
            if constexpr (Scalar<T> || Complex<T>)
                return fmt::format_to(ctx.out(), "{}", arg);
            else if constexpr (std::same_as<T, std::string>)
                return fmt::format_to(ctx.out(), "{}",  arg);
            else if constexpr (UniformVector<T> || UniformStringVector<T>)
                return fmt::format_to(ctx.out(), "[{}]", fmt::join(arg, ", "));
            else if constexpr (std::same_as<T, std::vector<pmt>>) {
                fmt::format_to(ctx.out(), "[");
                auto new_func = [&function](const auto new_arg) -> ret_type { return function(new_arg, function); };
                for (auto& a: std::span(arg).first(arg.size()-1)) {
                    std::visit(new_func, a);
                    fmt::format_to(ctx.out(), ", ");
                }
                std::visit(new_func, arg[arg.size()-1]);
                return fmt::format_to(ctx.out(), "]");
                // When we drop support for gcc11/clang15, get rid of the nested lambda and replace
                // the above with this line.
                //return fmt::format_to(ctx.out(), "[{}]", fmt::join(arg, ", "));
            } else if constexpr (PmtMap<T>) {
                fmt::format_to(ctx.out(), "{{");
                auto new_func = [&function](const auto new_arg) -> ret_type { return function(new_arg, function); };
                size_t i = 0;
                for (auto& [k, v]: arg) {
                    fmt::format_to(ctx.out(), "{}: ", k);
                    std::visit(new_func, v);
                    if (i++ < arg.size() - 1)
                        fmt::format_to(ctx.out(), ", ");
                }
                return fmt::format_to(ctx.out(), "}}");
                // When we drop support for gcc11/clang15, get rid of the nested lambda and replace
                // the above with this line.
                //return fmt::format_to(ctx.out(), "{{{}}}", fmt::join(arg, ", "));
            } else if constexpr (std::same_as<std::monostate, T>)
                return fmt::format_to(ctx.out(), "null");
            return fmt::format_to(ctx.out(), "unknown type {}", typeid(T).name());
            };
            return function_main(format_arg, function_main);
        };
        return std::visit(format_func, value);

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
