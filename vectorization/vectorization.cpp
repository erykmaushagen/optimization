#include <immintrin.h>
#include <perfcpp/event_counter.h>

#include <iostream>
#include <random>
#include <tuple>

enum BenchmarkType {
    SCALAR,
    VECTORIZED_SSE,
    VECTORIZED_OPITMIZED
};

int scalarSumWithMask(const uint32_t *indBonus, const int32_t *recBonus, size_t size) {
    uint64_t sum = 0;
    // TODO: Implement the following query logic
    //   SELECT SUM(Employees.IndividualBonus)
    //   FROM Employees
    //   WHERE Employees.ReceivedBonus = -1
    for (int i = 0; i < size; i++){
        if(recBonus[i] == -1) sum += indBonus[i];
    }
    return sum;
}

int sseSumWithMask(const uint32_t *indBonus, const int32_t *recBonus, std::size_t size) {
    
    __m128i sum = _mm_setzero_si128(); //  128-bit SIMD-Register auf zero mit32 bit lanes

    for (std::size_t i = 0; i + 4 <= size; i += 4){ // Idee: lade 8 lanes, statt nur ein array element zu betrachten
        __m128i ind = _mm_loadu_si128((const __m128i*)(indBonus + i));
        __m128i red = _mm_loadu_si128((const __m128i*)(recBonus + i));

        // erzeuge maske mit 128 bit auf 1
        __m128i mask = _mm_cmpeq_epi32(red, _mm_set1_epi32(-1));

        // bitwise-and mit gezeogenen 128 bit und mit 128 bit Maske auf 1 
        __m128i filtered = _mm_and_si128(ind, mask);

        // bilde summe für jede lane
        sum = _mm_add_epi32(sum, filtered);
    }
    // addiere lanes zusammen
    __m128i sum2 = _mm_hadd_epi32(sum, sum);
    sum2 = _mm_hadd_epi32(sum2, sum2);
    int result = _mm_cvtsi128_si32(sum2);

    for (std::size_t i = size - (size % 4); i < size; ++i) {
        if (recBonus[i] == -1) {
            result += indBonus[i];
        }
    }

    return result;
    
}

int optimizedSumWithMask(const uint32_t *indBonus, const int32_t *recBonus, size_t size) {

    __m256i sum = _mm256_setzero_si256(); // 256-bit SIMD-Register mit 8 x 32-bit Lanes

    for (std::size_t i = 0; i + 8 <= size; i += 8) {
        __m256i ind = _mm256_loadu_si256((const __m256i*)(indBonus + i));
        __m256i red = _mm256_loadu_si256((const __m256i*)(recBonus + i));

        // Erzeuge Maske: 0xFFFFFFFF, wenn rec == -1
        __m256i mask = _mm256_cmpeq_epi32(red, _mm256_set1_epi32(-1));

        // Bitwise AND mit Maske
        __m256i filtered = _mm256_and_si256(ind, mask);

        // Addiere die Werte laneweise
        sum = _mm256_add_epi32(sum, filtered);
    }

    // Addiere die 8 Lanes zusammen
    __m128i low = _mm256_castsi256_si128(sum);
    __m128i high = _mm256_extracti128_si256(sum, 1);

    __m128i sum2 = _mm_add_epi32(low, high);
    sum2 = _mm_hadd_epi32(sum2, sum2);
    sum2 = _mm_hadd_epi32(sum2, sum2);

    int result = _mm_cvtsi128_si32(sum2);

    // Verarbeite verbleibende Elemente
    for (std::size_t i = size - (size % 8); i < size; ++i) {
        if (recBonus[i] == -1) {
            result += indBonus[i];
        }
    }

    return result;
}

void compare_results(const uint32_t *indBonus, const int32_t *recBonus, const std::size_t elements) {
    // Print as CSV
    int scalarResult = scalarSumWithMask(indBonus, recBonus, elements);
    int sseResult = sseSumWithMask(indBonus, recBonus, elements);
    int optResult = optimizedSumWithMask(indBonus, recBonus, elements);

    std::vector<std::tuple<std::string, int>> results;
    if (scalarResult != 0) {
        results.push_back(std::make_tuple("scalar", scalarResult));
    }

    if (sseResult != 0) {
        results.push_back(std::make_tuple("sse", sseResult));
    }

    if (optResult != 0) {
        results.push_back(std::make_tuple("optimized", optResult));
    }

    if (results.size() > 1) {
        bool all_equal = true;
        for (size_t i = 1; i < results.size(); ++i) {
            if (std::get<1>(results[i]) != std::get<1>(results[0])) {
                all_equal = false;
                break;
            }
        }

        if (all_equal) {
            std::cerr << "All implemented results equal!" << std::endl;
        } else {
            std::cerr << "Results are different!" << std::endl;
            for (const auto &result : results) {
                std::cerr << std::get<0>(result) << ": " << std::get<1>(result) << std::endl;
            }
        }
    } else if (results.size() == 1) {
        std::cerr << "Only one result implemented!" << std::endl;
    } else {
        std::cerr << "No results implemented!" << std::endl;
    }

}

std::string run_benchmark(const uint32_t *indBonus, const int32_t *recBonus, const std::size_t elements, const BenchmarkType benchmark_type, const uint16_t iterations) {
    auto counter_definitions = perf::CounterDefinition{};
    auto config = perf::Config{};
    config.include_kernel(false);
    auto event_counter = perf::EventCounter{counter_definitions, config};

    try {
        event_counter.add({"cycles", "instructions", "cache-misses"});
    } catch (std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }

    try {
        event_counter.start();
    } catch (std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
    auto sum = 0;
    if (benchmark_type == SCALAR) {
        for (uint32_t i = 0; i < iterations; ++i) {
            sum = scalarSumWithMask(indBonus, recBonus, elements);
        }
    } else if (benchmark_type == VECTORIZED_SSE) {
        for (uint32_t i = 0; i < iterations; ++i) {
            sum = sseSumWithMask(indBonus, recBonus, elements);
        }
    } else if (benchmark_type == VECTORIZED_OPITMIZED) {
        for (uint32_t i = 0; i < iterations; ++i) {
            sum = optimizedSumWithMask(indBonus, recBonus, elements);
        }
    }

    event_counter.stop();

    /// Calculate the result.
    const auto result = event_counter.result();

    // Print as CSV
    std::string additional = "scalar " + std::to_string(sum);
    if (benchmark_type == VECTORIZED_SSE) {
        additional = "vectorized_sse " + std::to_string(sum);
    } else if (benchmark_type == VECTORIZED_OPITMIZED) {
        additional = "vectorized_optimized " + std::to_string(sum);
    }
    return result.to_csv(',', false, additional);
}

int main() {
    const uint32_t indBonus_size_in_mb = 1000;
    const uint64_t elements = (indBonus_size_in_mb * 1024 * 1024) / sizeof(uint32_t);
    const uint64_t iterations = 20;

    uint32_t *indBonus = new uint32_t[elements];
    int32_t *recBonus = new int32_t[elements];

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dataDist(0, 5000);
    std::uniform_int_distribution<int> maskDist(0, 1);

    for (size_t i = 0; i < elements; ++i) {
        indBonus[i] = dataDist(gen);
        recBonus[i] = maskDist(gen) ? -1 : 0;
    }

    compare_results(indBonus, recBonus, elements);

    // Führe die unterschiedlichen Benchmarkkonfigurationen aus
    std::cout << "counter,value,type" << std::endl;
    std::cout << run_benchmark(indBonus, recBonus, elements, SCALAR, iterations) << std::endl;
    std::cout << run_benchmark(indBonus, recBonus, elements, VECTORIZED_SSE, iterations) << std::endl;
    std::cout << run_benchmark(indBonus, recBonus, elements, VECTORIZED_OPITMIZED, iterations) << std::endl;

    // Gebe den reservierten Speicher wieder frei
    delete[] indBonus;
    delete[] recBonus;
}