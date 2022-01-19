#include <chrono>
#include <iostream>
#include <string>

#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"

#include <pmtf/base.hpp>
#include <pmtf/map.hpp>
#include <pmtf/scalar.hpp>

using namespace pmtf;

bool run_test(const int times, uint64_t nitems)
{

    bool valid = true;
    for (int i = 0; i < times; i++) {
        // Create the dictionary
        std::map<std::string, pmt> starting_map;

        #if 1
        for (uint64_t k = 0; k < nitems; k++) {
            auto key = std::string("key" + std::to_string(k));
            auto value = scalar(k);

            starting_map[key] = value;
        }
        auto d_in = map(starting_map);
        #else
        auto d_in = map<std::string>::make(starting_map);
        for (int k = 0; k < nitems; k++) {
            auto key = std::string("key" + std::to_string(k));
            auto value = scalar<int32_t>::make(k);

            d_in->set(key,value);
        }
        #endif

        #if 0
        auto d_out = d_in->value();

        for (int k = 0; k < nitems; k++) {
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
    uint64_t samples = 10000;
    uint64_t items = 100;


    CLI::App app{"Benchmarking Script for Dictionary Packing and Unpacking"};

    // app.add_option("-h,--help", "display help");
    app.add_option("--samples", samples, "Number of times to perform lookup");
    app.add_option("--items", items, "Number of items in dict");

    CLI11_PARSE(app, argc, argv);

    {

        auto t1 = std::chrono::steady_clock::now();

        auto valid = run_test(samples, items);

        auto t2 = std::chrono::steady_clock::now();
        auto time =
            std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() / 1e9;

        std::cout << "[PROFILE_TIME]" << time << "[PROFILE_TIME]" << std::endl;
        std::cout << "[PROFILE_VALID]" << valid << "[PROFILE_VALID]" << std::endl;
    }
}
