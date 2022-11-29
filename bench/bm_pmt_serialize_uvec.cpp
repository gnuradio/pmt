#include <chrono>
#include <iostream>

#include "CLI/App.hpp"
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"

#include <pmtv/pmt.hpp>

using namespace pmtv;

bool run_test(const int times, const std::vector<int32_t>& data)
{
    bool valid = true;

    std::stringbuf sb; // fake channel
    for (int i = 0; i < times; i++) {
        sb.str(""); // reset channel to empty
        // auto p1 = vector<int32_t>(data);
        pmt p1 = data;
        pmtv::serialize(sb, p1);
        auto p2 = pmtv::deserialize(sb);
        if (p1 != p2)
            valid = false;
    }

    return valid;
}

int main(int argc, char* argv[])
{
    uint64_t samples = 1000000;
    size_t veclen = 1024;

    CLI::App app{ "Benchmarking Script for Uniform Vector Serialization" };

    // app.add_option("-h,--help", "display help");
    app.add_option("--samples", samples, "Number of Samples");
    app.add_option("--veclen", veclen, "Vector Length");

    CLI11_PARSE(app, argc, argv);

    {

        std::vector<int32_t> data(veclen);
        for (size_t i = 0; i < veclen; i++) {
            data[i] = i;
        }
        auto t1 = std::chrono::steady_clock::now();

        auto valid = run_test(samples, data);

        auto t2 = std::chrono::steady_clock::now();
        auto time =
            std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() / 1e9;

        std::cout << "[PROFILE_TIME]" << time << "[PROFILE_TIME]" << std::endl;
        std::cout << "[PROFILE_VALID]" << valid << "[PROFILE_VALID]" << std::endl;
    }
}
