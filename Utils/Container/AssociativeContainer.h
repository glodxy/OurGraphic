//
// Created by 77205 on 2022/1/26.
//

#ifndef OUR_GRAPHIC_UTILS_CONTAINER_ASSOCIATIVECONTAINER_H_
#define OUR_GRAPHIC_UTILS_CONTAINER_ASSOCIATIVECONTAINER_H_
#include <vector>

namespace our_graph::utils {

template<typename Key, typename Value>
class AssociativeContainer {
  // We use a std::vector instead of a std::multimap because we don't expect many items
  // in the cache and std::multimap generates tons of code. Even with more items, we
  // could improve this trivially by using a sorted std::vector.
  using Container = std::vector<std::pair<Key, Value>>;
  Container mContainer;
 public:
  using iterator = typename Container::iterator;
  using const_iterator = typename Container::const_iterator;
  using key_type = typename Container::value_type::first_type;
  using value_type = typename Container::value_type::second_type;

  size_t size() const { return mContainer.size(); }
  iterator begin() { return mContainer.begin(); }
  const_iterator begin() const { return mContainer.begin(); }
  iterator end() { return mContainer.end(); }
  const_iterator end() const  { return mContainer.end(); }
  iterator erase(iterator it) {
    return mContainer.erase(it);
  }
  const_iterator find(key_type const& key) const {
    return const_cast<AssociativeContainer*>(this)->find(key);
  }
  iterator find(key_type const& key) {
    return std::find_if(mContainer.begin(), mContainer.end(), [&key](auto const& v) {
      return v.first == key;
    });
  }
  template<typename ... ARGS>
  void emplace(ARGS&&... args) {
    mContainer.emplace_back(std::forward<ARGS>(args)...);
  }
};

}
#endif //OUR_GRAPHIC_UTILS_CONTAINER_ASSOCIATIVECONTAINER_H_
