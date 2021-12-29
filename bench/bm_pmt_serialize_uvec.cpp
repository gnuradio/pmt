#include <chrono>
#include <iostream>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <pmtf/vector.hpp>
#include <pmtf/wrap.hpp>

using namespace pmtf;

bool run_test(const int times, const std::vector<int32_t>& data)
{
    bool valid = true;

    std::stringbuf sb; // fake channel
    for (int i=0; i< times; i++)
    {
        sb.str(""); // reset channel to empty
        // auto p1 = vector<int32_t>(data);
        pmt p1 = data;
        p1.serialize(sb);
        auto p2 = pmt::deserialize(sb);
        if (p1 != p2)
            valid = false;
    }

    return valid;
}

int main(int argc, char* argv[])
{
    uint64_t samples;
    size_t veclen;

    po::options_description desc("Basic Test Flow Graph");
    desc.add_options()("help,h", "display help")(
        "samples",
        po::value<uint64_t>(&samples)->default_value(1000000),
        "Number of samples")(
        "veclen", po::value<size_t>(&veclen)->default_value(1024), "Vector Length");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    {

        std::vector<int32_t> data(veclen);
        for (size_t i=0; i<veclen; i++)
        {
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
