#include <array>
#include <functional>
#include <iostream>

std::array<int, 100> b;
int cnt = 1;

int stirling2nd(int n, int k) {
    if (k >= n || k <= 1) {
        return 1;
    }
    return stirling2nd(n - 1, k - 1) + k * stirling2nd(n - 1, k);
}

void Move(int x, int y) {
    cnt += 1;
    std::cout << x << " " << y << std::endl;
}

void GEN1_even(int n, int k);
void NEG1_even(int n, int k);

void GEN0_odd(int n, int k) {
    if (k > 0 && k < n) {
        GEN1_even(n - 1, k - 1);
        Move(n - 1, k - 1);
        NEG1_even(n - 1, k);
        for (int i = k - 3; i >= 0; i -= 2) {
            Move(n, i);
            GEN1_even(n - 1, k);
            Move(n, i - 1);
            NEG1_even(n - 1, k);
        }
    }
}

void GEN1_even(int n, int k) {
    if (k > 1 && k < n) {
        NEG1_even(n - 1, k - 1);
        Move(n - 1, k - 2);
        GEN0_odd(n - 1, k);
        Move(n, k - 1);
        GEN1_even(n - 1, k - 2);
        for (int i = k - 4; i >= 0; i -= 2) {
            Move(n, i);
            NEG1_even(n - 1, k - 1);
            Move(n, i + 1);
            GEN0_odd(n - 1, k);
            Move(n, i);
            GEN1_even(n - 1, k - 2);
        }
    }
}

void NEG1_even(int n, int k) {
    if (k >= 0 && k < n - 1) {
        GEN1_even(n - 1, k);
        Move(n - 1, k);
        NEG1_even(n - 1, k + 1);
        for (int i = k + 2; i < n; i += 2) {
            Move(n - 1, i - 1);
            GEN1_even(n - 1, k);
            Move(n - 1, i);
            NEG1_even(n - 1, k + 1);
        }
    }
}

void set_partition(int n, int k) {
    if (k % 2 == 0) {
        GEN0_even(n, k);
    } else {
        GEN0_odd(n, k);
    }
}

int main() {
    int n = 5, k = 3;
    std::fill(b.begin(), b.begin() + n - k + 1, 0);
    for (int i = n - k + 1; i < n; i++) {
        b[i] = i - (n - k + 1);
    }
    set_partition(n, k);
    for (int i = 0; i < n; i++) {
        int old = b[i];
        b[i] = i;
        std::cout << "[";
        for (int j = 1; j < k; j++) {
            std::cout << b[j] << " ";
        }
        std::cout << "]: Move " << i << " from block " << old << " to " << b[i] << std::endl;
    }
    assert(stirling2nd(n, k) == cnt);
    std::cout << "Done." << std::endl;
    return 0;
}
