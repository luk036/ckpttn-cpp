#include <iostream>
#include <map>
#include <vector>

template <typename T> class Lict : public std::map<int, T> {
  public:
    Lict(std::vector<T> lst) {
        for (int i = 0; i < lst.size(); i++) {
            this->operator[](i) = lst[i];
        }
    }
    T &operator[](int key) { return std::map<int, T>::operator[](key); }
    const T &operator[](int key) const {
        return std::map<int, T>::operator[](key);
    }
    typename std::map<int, T>::iterator begin() {
        return std::map<int, T>::begin();
    }
    typename std::map<int, T>::iterator end() {
        return std::map<int, T>::end();
    }
    typename std::map<int, T>::const_iterator begin() const {
        return std::map<int, T>::begin();
    }
    typename std::map<int, T>::const_iterator end() const {
        return std::map<int, T>::end();
    }
    bool __contains__(int value) const {
        return this->find(value) != this->end();
    }
};

template <typename T> class ShiftArray : public std::vector<T> {
  public:
    ShiftArray() : start(0) {}
    T &operator[](int key) { return std::vector<T>::operator[](key - start); }
    const T &operator[](int key) const {
        return std::vector<T>::operator[](key - start);
    }
    typename std::vector<T>::iterator begin() {
        return std::vector<T>::begin();
    }
    typename std::vector<T>::iterator end() { return std::vector<T>::end(); }
    typename std::vector<T>::const_iterator begin() const {
        return std::vector<T>::begin();
    }
    typename std::vector<T>::const_iterator end() const {
        return std::vector<T>::end();
    }
    typename std::map<int, T>::iterator items() {
        std::map<int, T> result;
        for (int i = 0; i < this->size(); i++) {
            result[i + start] = this->operator[](i);
        }
        return result.begin();
    }

  private:
    int start;
};

int main() {
    std::vector<int> lst(8, 0);
    Lict<int> a(lst);
    for (auto &[key, value] : a) {
        value = key * key;
    }
    for (auto &[key, value] : a) {
        std::cout << key << ": " << value << std::endl;
    }
    std::cout << a.__contains__(3) << std::endl;
    return 0;
}
