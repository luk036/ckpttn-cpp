#include <iostream>
using namespace std;

template <typename T> class Dllink {
public:
  Dllink(T data) {
    next = prev = this;
    this->data = data;
  }
  void appendleft(Dllink<T> *node) {
    node->next = next;
    next->prev = node;
    next = node;
    node->prev = this;
  }
  Dllink<T> *next;
  Dllink<T> *prev;
  T data;
};

template <typename T> class DllIterator {
public:
  DllIterator(Dllink<T> *link) {
    this->link = link;
    cur = link->next;
  }
  Dllink<T> *next() {
    if (cur != link) {
      Dllink<T> *res = cur;
      cur = cur->next;
      return res;
    } else {
      throw "StopIteration";
    }
  }

private:
  Dllink<T> *link;
  Dllink<T> *cur;
};

template <typename T> class Dllist {
public:
  Dllist(T data) { head = new Dllink<T>(data); }
  bool is_empty() { return head->next == head && head->prev == head; }
  void appendleft(Dllink<T> *node) { head->appendleft(node); }
  Dllink<T> *pop() {
    Dllink<T> *node = head->prev;
    node->prev->next = head;
    head->prev = node->prev;
    return node;
  }
  Dllink<T> *popleft() {
    Dllink<T> *node = head->next;
    node->next->prev = head;
    head->next = node->next;
    return node;
  }
  DllIterator<T> *__iter__() { return new DllIterator<T>(head); }

private:
  Dllink<T> *head;
};

void test_dllink() {
  Dllist<int> *L1 = new Dllist<int>(99);
  Dllist<int> *L2 = new Dllist<int>(99);
  Dllink<int> *d = new Dllink<int>(1);
  Dllink<int> *e = new Dllink<int>(2);
  Dllink<int> *f = new Dllink<int>(3);
  L1->appendleft(e);
  if (!L1->is_empty()) {
    cout << "not empty" << endl;
  }
  L1->appendleft(f);
  L1->append(d);
  L2->append(L1->pop());
  L2->append(L1->popleft());
  if (!L1->is_empty()) {
    cout << "not empty" << endl;
  }
  if (!L2->is_empty()) {
    cout << "not empty" << endl;
  }
  e->prev->next = e->next;
  e->next->prev = e->prev;
  delete e;
  if (L1->is_empty()) {
    cout << "empty" << endl;
  }
  int count = 0;
  DllIterator<int> *iter = L2->__iter__();
  while (true) {
    try {
      iter->next();
      count++;
    } catch (const char *msg) {
      break;
    }
  }
  delete iter;
  if (count == 2) {
    cout << "count is 2" << endl;
  }
}

int main() {
  test_dllink();
  return 0;
}
