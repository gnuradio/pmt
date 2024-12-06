#include <chrono>
#include <iostream>
#include <string>

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push // ignore warning of external libraries that from this lib-context we do not have any control over
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wimplicit-int-float-conversion"
#ifndef __clang__ // only for GCC, not Clang
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
#endif

#include "CLI/App.hpp"
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#include <pmtv/pmt.hpp>
#include <pmtv/format.hpp>

using namespace pmtv;

bool run_test(const int32_t times, int32_t nitems)
{

    bool valid = true;
    for (int32_t i = 0; i < times; i++) {
        // Create the dictionary
        pmtv::map_t starting_map;

#if 1
        for (int32_t k = 0; k < nitems; k++) {
            auto key = fmt::format("key{}", k);
            auto value = pmt(k);

            starting_map[key] = value;
        }
        auto d_in = pmt(starting_map);
#else
        auto d_in = map<std::string>::make(starting_map);
        for (int32_t k = 0; k < nitems; k++) {
            auto key = std::string("key" + std::to_string(k));
            auto value = scalar<int32_t>::make(k);

            d_in->set(key, value);
        }
#endif

#if 0
        auto d_out = d_in->value();

        for (int32_t k = 0; k < nitems; k++) {
            auto key = std::string("key" + std::to_string(k));
            auto value = scalar<int32_t>::make(k);

            if (std::static_pointer_cast<scalar<int32_t>>(d_out[key])->value() != k) {
                valid = false;
            }
        }
#endif
    }
    return valid;
}

int main(int argc, char* argv[])
{
    int32_t samples = 10000;
    int32_t items = 100;


    CLI::App app{ "Benchmarking Script for Dictionary Packing and Unpacking" };

    // app.add_option("-h,--help", "display help");
    app.add_option("--samples", samples, "Number of times to perform lookup");
    app.add_option("--items", items, "Number of items in dict");

    CLI11_PARSE(app, argc, argv);

    {

        auto t1 = std::chrono::steady_clock::now();

        auto valid = run_test(samples, items);

        auto t2 = std::chrono::steady_clock::now();
        auto time = static_cast<long double>(1e-9) * static_cast<long double>(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());

        std::cout << "[PROFILE_TIME]" << time << "[PROFILE_TIME]" << std::endl;
        std::cout << "[PROFILE_VALID]" << valid << "[PROFILE_VALID]" << std::endl;
    }
}
