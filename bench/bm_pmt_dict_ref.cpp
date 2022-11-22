#include <chrono>
#include <iostream>
#include <string>

#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"

#include <pmtv/pmt.hpp>

using namespace pmtv;

bool run_test(const int times, pmt& d, int32_t index)
{
    std::stringbuf sb; // fake channel

    auto key = std::string("key"+std::to_string(index));

    auto themap = pmtv::get_map(d);

    bool valid = true;
    for (int i=0; i< times; i++)
    {
        auto ref = themap[key];

        // if (ref == nullptr)
        //    valid = false;
        auto s = pmtv::cast<int32_t>(ref);
        if (s != index)
        {
            valid = false;
        }
    }
    return valid;
}

int main(int argc, char* argv[])
{
    uint64_t samples = 10000;
    uint64_t items = 100;
    uint64_t index = 0;


    CLI::App app{"Benchmarking Script for Dictionary Packing and Unpacking"};

    // app.add_option("-h,--help", "display help");
    app.add_option("--samples", samples, "Number of times to perform lookup");
    app.add_option("--items", items, "Number of items in dict");
    app.add_option("--index", index, "Index for lookup");

    CLI11_PARSE(app, argc, argv);

    {
        // Create the dictionary
        std::map<std::string,pmt> starting_map;
        for (uint32_t k = 0; k < items; k++)
        {
            // auto key = std::string("key" + std::to_string(k));
            // auto value = scalar(k);

            starting_map["key" + std::to_string(k)] = pmt(k);
        }

        auto d = pmt(starting_map);

        auto t1 = std::chrono::steady_clock::now();

        auto valid = run_test(samples, d, index);

        auto t2 = std::chrono::steady_clock::now();
        auto time =
            std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() / 1e9;

        std::cout << "[PROFILE_TIME]" << time << "[PROFILE_TIME]" << std::endl;
        std::cout << "[PROFILE_VALID]" << valid << "[PROFILE_VALID]" << std::endl;
    }
}
