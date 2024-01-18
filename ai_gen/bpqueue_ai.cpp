class BPQueue {
  private:
    std::vector<Dllist<std::vector<int>>> _bucket;
    int _max, _offset, _high;

  public:
    BPQueue(int a, int b) {
        assert(a <= b);
        _max = 0;
        _offset = a - 1;
        _high = b - _offset;
        _bucket = std::vector<Dllist<std::vector<int>>>(
            this->_high + 1, Dllist<std::vector<int>>({std::vector<int>{i, 4848}}));
        _bucket[0].push_back(sentinel);
    }
    void clear() {
        while (_max > 0) {
            _bucket[_max].clear();
            _max -= 1;
        }
    }
    void append(Item it, int k) {
        assert(k > _offset);
        it[0] = k - _offset;
        if (_max < it[0]) {
            _max = it[0];
        }
        _bucket[it[0]].push_back(it);
    }
    class BPQueueIterator {
      private:
        BPQueue *bpq;
        int curkey;
        typename Dllist<std::vector<int>>::iterator curitem;

      public:
        BPQueueIterator(BPQueue *bpq) {
            this->bpq = bpq;
            this->curkey = bpq->_max;
            this->curitem = bpq->_bucket[bpq->_max].begin();
        }
        Item operator*() { return *curitem; }
        BPQueueIterator &operator++() {
            while (curkey > 0) {
                try {
                    ++curitem;
                    return *this;
                } catch (std::exception &e) {
                    curkey -= 1;
                    curitem = bpq->_bucket[curkey].begin();
                }
            }
            throw std::out_of_range("BPQueueIterator out of range");
        }
        bool operator!=(const BPQueueIterator &other) const {
            return curkey != other.curkey || curitem != other.curitem;
        }
    };
    BPQueueIterator begin() { return BPQueueIterator(this); }
    BPQueueIterator end() { return BPQueueIterator(nullptr); }
};
