#include <iostream>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <random>


int main(int argc, char const *argv[])
{
    int N = 10'000'000;
    std::vector<char> keys;
    keys.reserve(12);
    keys.push_back('P');
    keys.push_back('R');
    keys.push_back('N');
    keys.push_back('B');
    keys.push_back('Q');
    keys.push_back('K');
    keys.push_back('p');
    keys.push_back('r');
    keys.push_back('n');
    keys.push_back('b');
    keys.push_back('q');
    keys.push_back('k');


    std::vector<char> att_keys;
    att_keys.reserve(N);

    std::mt19937 rng(42); // deterministyczny RNG
    std::uniform_int_distribution<int> dist(0, 12 - 1);

    for(int i = 0; i < N; i++)
        att_keys.push_back( keys[dist(rng)] );
    

    int arr[107] = {0};
    arr['P'] = 0;
    arr['R'] = 1;
    arr['N'] = 2;
    arr['B'] = 3;
    arr['Q'] = 4;
    arr['K'] = 5;
    arr['p'] = 6;
    arr['r'] = 7;
    arr['n'] = 8;
    arr['b'] = 9;
    arr['q'] = 10;
    arr['k'] = 11;

    std::unordered_map<char, int> map;
    map['P'] = 0;
    map['R'] = 1;
    map['N'] = 2;
    map['B'] = 3;
    map['Q'] = 4;
    map['K'] = 5;
    map['p'] = 6;
    map['r'] = 7;
    map['n'] = 8;
    map['b'] = 9;
    map['q'] = 10;
    map['k'] = 11;

    volatile long long sum = 0; // żeby kompilator nie zoptymalizował
    // --- TEST UNORDERED_MAP ---
    auto start = std::chrono::high_resolution_clock::now();
    // auto start = std::chrono::high_resolution_clock::now();
    for (int k : att_keys) {
        sum += map[k];
    }
    auto end = std::chrono::high_resolution_clock::now();
    // auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Unordered_map: "
              << std::chrono::duration<double, std::micro>(end - start).count()
              << " us\n";

    // --- TEST TABLICY ---
    start = std::chrono::high_resolution_clock::now();
    // auto start = std::chrono::high_resolution_clock::now();
    for (int k : att_keys) {
        sum += arr[k];
    }
    end = std::chrono::high_resolution_clock::now();
    // auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Array: "
              << std::chrono::duration<double, std::micro>(end - start).count()
              << " us\n";
              
    

              
    std::cout << "Ignore: " << sum << "\n"; // żeby sum nie znikło
    return 0;
}
