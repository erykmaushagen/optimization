#include <immintrin.h>
#include <perfcpp/event_counter.h>

#include <iostream>
#include <random>
#include <tuple>
#include <thread>
#include <mutex>

uint32_t filtered_bonus(uint32_t &bonus) {
    if (bonus < 3000) {
        return bonus - (bonus / 3);
    }
    return bonus - (bonus * 3);
}

int baseline_bonus_error(uint32_t *allBonuses, const uint32_t *wrongBonus, size_t wrongBonusSize) {
    uint64_t sum = 0;
    for(size_t i=0; i<wrongBonusSize; ++i){
        uint32_t bonus = filtered_bonus(allBonuses[wrongBonus[i]]);
        sum += bonus;
    }
    return sum;
}

int multi_bonus_error(uint32_t *allBonuses, const uint32_t *wrongBonus, size_t wrongBonusSize) {
    
    return 0;
}

bool run_benchmark(uint32_t *allBonuses, const uint32_t *wrongBonus, const std::size_t elements, const uint16_t iterations) {
    auto counter_definitions = perf::CounterDefinition{"../events.csv"};
    auto config = perf::Config{};
    config.include_kernel(false);
    auto event_counter = perf::EventCounter{counter_definitions, config};

    try {
        event_counter.add({"cycles", "instructions", "cache-misses"});
    } catch (std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }

    auto sum_baseline = 0;
    auto sum_multi = 0;
    try {
        event_counter.start();
    } catch (std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
    for (uint32_t i = 0; i < iterations; ++i) {
        sum_baseline = baseline_bonus_error(allBonuses, wrongBonus, elements);
    }
    event_counter.stop();

    const auto result_baseline = event_counter.result();

    try {
        event_counter.start();
    } catch (std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
    for (uint32_t i = 0; i < iterations; ++i) {
        sum_multi = multi_bonus_error(allBonuses, wrongBonus, elements);
    }
    event_counter.stop();

    /// Calculate the result.
    const auto result_multi = event_counter.result();

    
    // Print as CSV
    std::string additional = "baseline";
    std::cout << result_baseline.to_csv(',', true, additional) << std::endl;
    
    additional = "multi threaded";
    std::cout << result_multi.to_csv(',', false, additional) << std::endl;

    if (sum_baseline != sum_multi) {
        std::cerr << "Sum results are not the same, baseline is " << sum_baseline << ", multi threaded is " << sum_multi << std::endl;
        return false;
    }

    return true;
}

int main() {
    const uint32_t allBonuses_size_in_mb = 1000;
    const uint64_t elements = (allBonuses_size_in_mb * 1024 * 1024) / sizeof(uint32_t);
    const uint64_t iterations = 20;
    const uint64_t recElements = elements / 10;

    uint32_t *allBonuses = new uint32_t[elements];
    uint32_t *wrongBonus = new uint32_t[recElements];

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dataDist(0, 5000);
    std::uniform_int_distribution<uint32_t> maskDist(0, elements);

    for (size_t i = 0; i < elements; ++i) {
        allBonuses[i] = dataDist(gen);
    }

    for (size_t i = 0; i < recElements; ++i) {
        wrongBonus[i] = maskDist(gen);
    }

    // Führe die unterschiedlichen Benchmarkkonfigurationen aus
    auto sameResult = run_benchmark(allBonuses, wrongBonus, recElements, iterations);
    if (!sameResult) {
       return 1;
    }

    // Gebe den reservierten Speicher wieder frei
    delete[] allBonuses;
    delete[] wrongBonus;

    return 0;
}