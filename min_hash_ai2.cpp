#include <algorithm>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

class MinHash {
public:
  // Constructor
  MinHash(int k, int seed = 10) {
    this->k = k;       // Set the number of hash functions
    this->seed = seed; // Set the seed for the random number generator

    // Initialize the random number generator
    std::mt19937_64 rng(seed);

    // Generate k random masks
    masks.resize(k);
    for (int i = 0; i < k; i++) {
      masks[i] =
          rng(); // Use the random number generator to get a 64-bit integer
    }

    // Initialize the hashes vector with the maximum int64 value
    hashes.resize(k, std::numeric_limits<int64_t>::max());
  }

  // Add a value to the minhash
  void add(int64_t v) {
    // Compute the hashes for the value using the masks
    std::vector<int64_t> hashes_v(k);
    for (int i = 0; i < k; i++) {
      hashes_v[i] =
          masks[i] ^ std::hash<int64_t>()(v); // Use bitwise XOR to combine the
                                              // mask and the hash of the value
    }

    // Update the hashes vector with the minimum hashes
    for (int i = 0; i < k; i++) {
      hashes[i] = std::min(
          hashes[i],
          hashes_v[i]); // Use std::min to compare and update the hashes
    }
  }

  // Calculate the Jaccard similarity with another minhash
  double jaccard(MinHash &other) {
    // Check if the masks are the same
    for (int i = 0; i < k; i++) {
      if (masks[i] != other.masks[i]) {
        throw std::runtime_error("Can only calculate similarity "
                                 "between MinHashes with the same hash "
                                 "functions.");
      }
    }

    // Count the number of equal hashes
    int count = 0;
    for (int i = 0; i < k; i++) {
      if (hashes[i] == other.hashes[i]) {
        count++; // Increment the count if the hashes are equal
      }
    }

    // Return the Jaccard similarity
    return static_cast<double>(count) /
           k; // Divide the count by k and cast it to double
  }

private:
  int k;                       // The number of hash functions
  int seed;                    // The seed for the random number generator
  std::vector<int64_t> masks;  // The random masks for hashing
  std::vector<int64_t> hashes; // The minimum hashes for each hash function
};
