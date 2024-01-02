#include <chrono>
#include <iostream>
#include <string>

#include "CLI/App.hpp"
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"

#include <pmtv/pmt.hpp>

using namespace pmtv;

bool run_test(const int32_t times, pmt& d, int32_t index)
{
    std::stringbuf sb; // fake channel

    auto key = fmt::format("key{}", index);

    auto themap = pmtv::get_map(d);

    bool valid = true;
    for (int32_t i = 0; i < times; i++) {
        auto ref = themap[key];

        // if (ref == nullptr)
        //    valid = false;
        auto s = pmtv::cast<int32_t>(ref);
        if (s != index) {
            valid = false;
        }
    }
    return valid;
}

int main(int argc, char* argv[])
{
    int32_t samples = 10000;
    int32_t items = 100;
    int32_t index = 0;


    CLI::App app{ "Benchmarking Script for Dictionary Packing and Unpacking" };

    // app.add_option("-h,--help", "display help");
    app.add_option("--samples", samples, "Number of times to perform lookup");
    app.add_option("--items", items, "Number of items in dict");
    app.add_option("--index", index, "Index for lookup");

    CLI11_PARSE(app, argc, argv);

    {
        // Create the dictionary
        std::map<std::string, pmt> starting_map;
        for (int32_t k = 0; k < items; k++) {
            // auto key = std::string("key" + std::to_string(k));
            // auto value = scalar(k);

            starting_map["key" + std::to_string(k)] = pmt(k);
        }

        auto d = pmt(starting_map);

        auto t1 = std::chrono::steady_clock::now();

        auto valid = run_test(samples, d, index);

        auto t2 = std::chrono::steady_clock::now();
        auto time = 1e-9 * static_cast<long double>(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());

        std::cout << "[PROFILE_TIME]" << time << "[PROFILE_TIME]" << std::endl;
        std::cout << "[PROFILE_VALID]" << valid << "[PROFILE_VALID]" << std::endl;
    }
}
