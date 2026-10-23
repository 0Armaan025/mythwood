

#include "../include/Inventory.h"

#include <algorithm>

bool Inventory::addItem(const Item &item) {
  if (item.quantity <= 0 || item.maxStack <= 0)
    return false;

  int remaining = item.quantity;

  // Fill existing stacks first.
  for (auto &existing : items) {
    if (existing.id != item.id)
      continue;

    const int capacity = existing.maxStack - existing.quantity;
    const int amount = min(capacity, remaining);

    if (amount > 0) {
      existing.quantity += amount;
      remaining -= amount;
    }

    if (remaining == 0)
      return true;
  }

  // Add new stacks if slots are available.
  while (remaining > 0) {
    if (items.size() >= MAX_SLOTS)
      return false;

    const int amount = min(item.maxStack, remaining);

    items.push_back({item.id, item.name, amount, item.maxStack});

    remaining -= amount;
  }

  return true;
}

bool Inventory::removeItem(const string &id, int quantity) {
  if (quantity <= 0 || countItem(id) < quantity)
    return false;

  int remaining = quantity;

  for (auto it = items.begin(); it != items.end();) {
    if (it->id != id) {
      ++it;
      continue;
    }

    const int amount = min(it->quantity, remaining);

    it->quantity -= amount;
    remaining -= amount;

    if (it->quantity == 0)
      it = items.erase(it);
    else
      ++it;

    if (remaining == 0)
      break;
  }

  return true;
}

int Inventory::countItem(const string &id) const {
  int total = 0;

  for (const auto &item : items) {
    if (item.id == id)
      total += item.quantity;
  }

  return total;
}

const vector<Item> &Inventory::getItems() const { return items; }
