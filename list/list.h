#pragma once
#include <iterator>
#include <memory>

namespace task {

template<class T, class Alloc = std::allocator<T>>
class list {
  struct Node {
    template<typename... Args>
    Node(Node *prev, Node *next, Args &&... args)
        : m_val(std::forward<Args>(args)...), m_prev(prev), m_next(next) {
    }

    T m_val;
    Node *m_prev;
    Node *m_next;
  };

  using NodeAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;

 public:
  class iterator {
   public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T *;
    using reference = T &;
    using iterator_category = std::bidirectional_iterator_tag;

    iterator()
        : m_node(nullptr) {
    }

    explicit iterator(Node *node)
        : m_node(node) {
    }

    iterator(const iterator &other) {
      *this = other;
    }

    iterator &operator=(const iterator &other) {
      if (this == &other)
        return *this;

      this->m_node = other.m_node;
      return *this;
    }

    iterator &operator++() {
      m_node = m_node->m_next;
      return *this;
    }

    iterator operator++(int) {
      auto tmp = *this;
      m_node = m_node->m_next;
      return tmp;
    }

    reference operator*() const {
      return m_node->m_val;
    }

    pointer operator->() const {
      return &m_node->m_val;
    }

    iterator &operator--() {
      m_node = m_node->m_prev;
      return *this;
    }

    iterator operator--(int) {
      auto tmp = *this;
      m_node = m_node->m_prev;
      return tmp;
    }

    bool operator==(iterator other) const {
      return m_node == other.m_node;
    }

    bool operator!=(iterator other) const {
      return m_node != other.m_node;
    }

   private:
    Node *m_node;

    friend class list;
  };

  class const_iterator {
   public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = const T *;
    using reference = const T &;
    using iterator_category = std::bidirectional_iterator_tag;

    const_iterator()
        : m_node(nullptr) {
    }

    explicit const_iterator(Node *node)
        : m_node(node) {
    }

    const_iterator(const iterator &other) {
      *this = other;
    }

    const_iterator &operator=(const iterator &other) {
      this->m_node = other.m_node;
      return *this;
    }

    const_iterator &operator++() {
      m_node = m_node->m_next;
      return *this;
    }

    const_iterator operator++(int) {
      auto tmp = *this;
      m_node = m_node->m_next;
      return tmp;
    }

    reference operator*() const {
      return m_node->m_val;
    }

    pointer operator->() const {
      return &m_node->m_val;
    }

    const_iterator &operator--() {
      m_node = m_node->m_prev;
      return *this;
    }

    const_iterator operator--(int) {
      auto tmp = *this;
      m_node = m_node->m_prev;
      return tmp;
    }

    bool operator==(const_iterator other) const {
      return m_node == other.m_node;
    }

    bool operator!=(const_iterator other) const {
      return m_node != other.m_node;
    }

    iterator drop_const() const {
      return iterator(const_cast<Node *>(m_node));
    }

   private:
    Node *m_node;

    friend class list;
  };

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  list()
      : m_node_alloc() {
    init();
  }

  explicit list(const Alloc &alloc)
      : m_node_alloc(alloc) {
    init();
  }

  list(size_t count, const T &value, const Alloc &alloc = Alloc())
      : list(alloc) {
    for (; count; --count) {
      push_back(value);
    }
  }

  explicit list(size_t count, const Alloc &alloc = Alloc())
      : list(alloc) {
    for (; count; --count) {
      emplace_back();
    }
  }

  ~list() {
    clear();

    if (m_end) // move constructor might have stolen from us: ugly design
      m_node_alloc.deallocate(m_end, 1);
  }

  list(const list &other)
      : m_node_alloc(other.get_allocator()) {
    init();

    auto first = other.cbegin();
    auto end = other.cend();

    for (; first != end; ++first) {
      push_back(*first);
    }
  }

  list(list &&other) {
    init();
    swap(other);
    // leave allocator the same
  }

  list &operator=(const list &other) {
    if (&other == this)
      return *this;

    list new_self{other};
    swap(new_self);
    return *this;
  }

  list &operator=(list &&other) noexcept {
    if (&other == this)
      return *this;

    clear();
    swap(other);
    return *this;
  }

  Alloc get_allocator() const {
    return Alloc(m_node_alloc);
  }

  T &front() {
    return m_head->m_val;
  }

  const T &front() const {
    return m_head->m_val;
  }

  T &back() {
    return *(--end());
  }

  const T &back() const {
    return *(--cend());
  };

  iterator begin() {
    return iterator(m_head);
  }

  iterator end() {
    return iterator(m_end);
  }

  const_iterator cbegin() const {
    return const_iterator(m_head);
  }

  const_iterator cend() const {
    return const_iterator(m_end);
  }

  reverse_iterator rbegin() {
    return reverse_iterator(end());
  }

  reverse_iterator rend() {
    return reverse_iterator(begin());
  }

  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(cend());
  }

  const_reverse_iterator crend() const {
    return const_reverse_iterator(cbegin());
  }

  bool empty() const {
    return m_size == 0;
  }

  size_t size() const {
    return m_size;
  }

  size_t max_size() const {
    return m_size;
  }

  void clear() {
    Node *cur = m_head;

    while (cur != m_end) {
      Node *next = cur->m_next;
      m_node_alloc.destroy(cur);
      m_node_alloc.deallocate(cur, 1);
      cur = next;
    }

    m_head = m_end;
    m_size = 0;
  }

  iterator insert(const_iterator pos, const T &value) {
    Node *new_node = create_node(value);
    insert_node(pos, new_node);

    return iterator(new_node);
  }

  iterator insert(const_iterator pos, T &&value) {
    Node *new_node = create_node(std::move(value));
    insert_node(pos, new_node);

    return iterator(new_node);
  }

  iterator insert(const_iterator pos, size_t count, const T &value) {
    Node *node = pos.m_node;

    for (; count; --count) {
      node = create_node(value);
      insert_node(pos, node);
    }

    return iterator(node);
  }

  iterator erase(const_iterator pos) {
    Node *gone = take_node(pos);
    Node *ret = gone->m_next;

    m_node_alloc.destroy(gone);
    m_node_alloc.deallocate(gone, 1);

    if (m_size == 0)
      m_head = m_end;

    return iterator(ret);
  }

  iterator erase(const_iterator first, const_iterator last) {
    while (first != last)
      first = erase(first);

    return last.drop_const();
  }

  void push_back(const T &value) {
    insert(end(), value);
  }

  void push_back(T &&value) {
    insert(end(), std::move(value));
  }

  void pop_back() {
    erase(--end());
  }

  void push_front(const T &value) {
    insert(begin(), value);
  }

  void push_front(T &&value) {
    insert(begin(), std::move(value));
  }

  void pop_front() {
    erase(begin());
  }

  template<class... Args>
  iterator emplace(const_iterator pos, Args &&... args) {
    Node *new_node = create_node(std::forward<Args>(args)...);
    insert_node(pos, new_node);

    return iterator(new_node);
  }

  template<class... Args>
  void emplace_back(Args &&... args) {
    Node *new_node = create_node(std::forward<Args>(args)...);
    insert_node(end(), new_node);
  }

  template<class... Args>
  void emplace_front(Args &&... args) {
    Node *new_node = create_node(std::forward<Args>(args)...);
    insert_node(begin(), new_node);
  }

  void resize(size_t count) {
    if (count < m_size) {
      size_t erase_count = m_size - count;
      for (; erase_count; --erase_count)
        pop_back();
    } else if (count > m_size) {
      size_t add_count = count - m_size;
      for (; add_count; --add_count)
        emplace_back();
    }
  }

  void swap(list &other) {
    std::swap(m_node_alloc, other.m_node_alloc);
    std::swap(other.m_head, m_head);
    std::swap(other.m_size, m_size);
    std::swap(other.m_end, m_end);
  }

  void merge(list &other) {
    if (&other == this)
      return;

    list<T> merged;
    auto it1 = begin();
    auto it2 = other.begin();
    while (it1 != end() || it2 != other.end()) {
      if (it1 == end())
        merged.insert_node(merged.end(), other.take_node(it2++));
      else if (it2 == other.end())
        merged.insert_node(merged.end(), take_node(it1++));
      else if (*it1 < *it2)
        merged.insert_node(merged.end(), take_node(it1++));
      else
        merged.insert_node(merged.end(), other.take_node(it2++));
    }

    *this = std::move(merged);
  }

  void splice(const_iterator pos, list &other) {
    if (other.empty())
      return;

    Node *cur_node = pos.m_node;

    if (pos == begin()) {
      m_head = other.m_head;
    } else {
      cur_node->m_prev->m_next = other.m_head;
    }

    other.m_end->m_prev->m_next = cur_node;

    m_size += other.m_size;

    other.m_size = 0;
    other.m_head = other.m_end;
  }

  void remove(const T &value) {
    iterator it = begin();
    iterator remove_later = end();

    while (it != end()) {
      iterator next = it;
      ++next;

      if (*it == value) {
        if (std::addressof(value) != std::addressof(*it))
          erase(it);
        else
          remove_later = it;
      }

      it = next;
    }

    if (remove_later != end())
      erase(remove_later);
  }

  void reverse() {
    if (m_size == 0)
      return;

    Node *old_head = m_head;
    m_head = m_end->m_prev;

    Node *cur = m_end->m_prev;
    while (cur != m_end) {
      std::swap(cur->m_next, cur->m_prev);
      if (cur == old_head)
        cur->m_next = m_end;
      cur = cur->m_next;
    }

    m_end->m_prev = old_head;
  }

  void unique() {
    if (m_size < 2)
      return;

    auto it = begin();
    Node *prev_node = it.m_node;
    ++it;

    while (it != end()) {
      if (it.m_node->m_val == prev_node->m_val)
        it = erase(it);
      else {
        prev_node = it.m_node;
        ++it;
      }
    }
  }

  void sort() {
    if (this->size() <= 1)
      return;

    list<T> left;
    list<T> right;

    auto start_size = size();
    for (auto it = begin(); it != end();) {
      auto next = std::next(it);
      if (size() > start_size / 2)
        left.insert_node(left.end(), take_node(begin()));
      else
        right.insert_node(right.end(), take_node(begin()));
      it = next;
    }

    left.sort();
    right.sort();

    left.merge(right);
    this->swap(left);
  }

 private:
  void init() {
    m_size = 0;
    m_end = m_node_alloc.allocate(1);
    m_head = m_end;
  }

  struct node_guard {
    node_guard(NodeAlloc &alloc, Node *node)
        : m_alloc(alloc), m_node(node), m_need_free(true) {
    }

    ~node_guard() {
      if (m_need_free)
        m_alloc.deallocate(m_node, 1);
    }

    void reset() {
      m_need_free = false;
    }

    NodeAlloc &m_alloc;
    Node *m_node;
    bool m_need_free;
  };

  template<typename... Args>
  Node *create_node(Args &&... args) {
    Node *new_node = m_node_alloc.allocate(1);
    node_guard guard(m_node_alloc, new_node);
    m_node_alloc.construct(&new_node->m_val, std::forward<Args>(args)...);
    guard.reset();
    return new_node;
  }

  Node *take_node(const_iterator pos) {
    Node *gone = pos.m_node;

    if (gone == m_head) {
      // update head
      m_head = gone->m_next;
    } else {
      // unlink gone node from siblings
      Node *prev = gone->m_prev;
      Node *next = gone->m_next;

      prev->m_next = next;
      next->m_prev = prev;
    }
    --m_size;
    return gone;
  }

  void insert_node(const_iterator pos, Node *new_node) {
    if (pos == begin() || m_size == 0) {
      Node *old_head = m_head;
      m_head = new_node;

      if (m_size == 0) {
        new_node->m_next = m_end;
        m_end->m_prev = new_node;
      } else {
        new_node->m_next = old_head;
        old_head->m_prev = new_node;
      }
    } else {
      Node *prev = pos.m_node->m_prev;
      Node *cur = pos.m_node;

      prev->m_next = new_node;
      new_node->m_prev = prev;
      new_node->m_next = cur;
      cur->m_prev = new_node;
    }

    ++m_size;
  }

 private:
  NodeAlloc m_node_alloc;

  Node *m_head;
  Node *m_end;
  size_t m_size;
};

} // namespace task
