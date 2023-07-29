#include <random>
#include <vector>

class MinHash {
  public:
    // Constructor
    MinHash(int k, int seed = 10) {
        // Initialize the class variables
        _k = k;
        _seed = seed;
        _masks = std::vector<int64_t>(k);
        // Generate k random numbers
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(
            std::numeric_limits<int64_t>::min(),
            std::numeric_limits<int64_t>::max());
        for (int i = 0; i < k; i++) {
            _masks[i] = dis(gen);
        }
        // Initialize the hashes vector to the maximum value
        _hashes = std::vector<int64_t>(k);
        std::fill(_hashes.begin(), _hashes.end(),
                  std::numeric_limits<int64_t>::max());
    }

    // Add a document to the MinHash signature
    void add(const std::string &v) {
        // Calculate the hashes for the document
        hashes = std::vector<int64_t>(k);
        hashes = std::bit_xor(_masks, hash(v));
        // Update the minimum hashes
        _hashes = std::min(_hashes, hashes);
    }

    // Calculate the Jaccard similarity between two MinHash signatures
    double jaccard(const MinHash &other) const {
        // Check that the hash functions are the same
        if (!std::equal(_masks.begin(), _masks.end(), other._masks.begin())) {
            throw std::exception(
                "Can only calculate similarity between MinHashes "
                "with the same hash functions.");
        }
        // Calculate the Jaccard similarity
        return std::accumulate(_hashes.begin(), _hashes.end(), 0) / float(_k);
    }

  private:
    // The number of hash functions
    int _k;
    // The seed for the random number generator
    int _seed;
    // The hash functions
    std::vector<int64_t> _masks;
    // The minimum hashes
    std::vector<int64_t> _hashes;
};
