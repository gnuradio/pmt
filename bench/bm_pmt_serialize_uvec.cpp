#include <chrono>
#include <iostream>

#include "CLI/App.hpp"
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"

#include <pmtv/pmt.hpp>

using namespace pmtv;

bool run_test(const int32_t times, const std::vector<int32_t>& data)
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
    int32_t samples = 1000000;
    std::size_t veclen = 1024;

    CLI::App app{ "Benchmarking Script for Uniform Vector Serialization" };

    // app.add_option("-h,--help", "display help");
    app.add_option("--samples", samples, "Number of Samples");
    app.add_option("--veclen", veclen, "Vector Length");

    CLI11_PARSE(app, argc, argv);

    {
        std::vector<int32_t> data(veclen);
        std::iota(data.begin(), data.end(), 0);

        auto t1 = std::chrono::steady_clock::now();

        auto valid = run_test(samples, data);

        auto t2 = std::chrono::steady_clock::now();
        auto time = 1e-9 * static_cast<long double>(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());

        std::cout << "[PROFILE_TIME]" << time << "[PROFILE_TIME]" << std::endl;
        std::cout << "[PROFILE_VALID]" << valid << "[PROFILE_VALID]" << std::endl;
    }
}
