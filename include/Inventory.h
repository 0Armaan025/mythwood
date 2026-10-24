#pragma once

#include <iostream>
#include <string>
#include <vector>

using namespace std;

struct Item {
  string id;
  string name;
  int quantity;
  int maxStack = 99;
};

class Inventory {
public:
  bool addItem(const Item &item);
  bool removeItem(const std::string &id, int quantity);
  int countItem(const std::string &id) const;

  const std::vector<Item> &getItems() const;

  bool setItems(const std::vector<Item> &loadedItems);

private:
  vector<Item> items;
  static constexpr int MAX_SLOTS = 20;
};
