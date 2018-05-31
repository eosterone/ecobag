#include <ecobag.hpp>
#include <eosiolib/print.hpp>

using namespace eosio;

void ecobag::createprofile(const account_name account, const string& username, const string& address) {
    
  eosio::require_auth(account); // optional: , N(active)

  // ??? Why define it everytime, cant this be a member?
  profile_table profiles(_self, _self);

  auto itr = profiles.find(account);
  eosio_assert(itr == profiles.end(), 
                      "Profile already exists");

  // 
  profiles.emplace(account, [&](auto& p) {
      p.account = account;
      p.username = username;
      p.address = address;
  });

  eosio::print("Profile created: ", name{account});
}

void ecobag::updateprofile(const account_name account, const string& username, const string& address) {
    
  eosio::require_auth(account); // optional: , N(active)

  // ??? Why do this everytime, cant this be a member?
  profile_table profiles(_self, _self);

  auto itr = profiles.find(account);
  eosio_assert(itr != profiles.end(), 
                      "Profile not found");

  profiles.modify(itr, account, [&](auto& p) {
      p.account = account;
      p.username = username;
      p.address = address;
  });

  eosio::print("Product updated");
}

void ecobag::removeprofile(const account_name account) {
  eosio::require_auth(account);

  profile_table profiles(_self, _self);

  auto itr = profiles.find(account);
  eosio_assert(itr != profiles.end(), 
                      "Profile does not exist");

  profiles.erase(itr);

  eosio::print("Profile deleted: ", name{account});
}

void ecobag::createitem(const account_name store, uint64_t sku, const string& commonname, uint32_t price, uint32_t quantity) {
  eosio::require_auth(store);

  item_table inventory(_self, _self);

  auto itr = inventory.find(sku);
  eosio_assert(itr == inventory.end(), 
                      "Product SKU already exists, use addproduct() to update quantity");

  // eosio_assert(quantity > 0);
  inventory.emplace(store, [&](auto& item) {
      item.sku = sku;
      item.store = store;
      item.commonname = commonname;
      item.price = price;
      item.quantity = quantity;
  });

  eosio::print("Product created: ", sku);
}

void ecobag::updateitem(const account_name store, uint64_t sku, const string& commonname, uint32_t price) {
  eosio::require_auth(store);

  item_table inventory(_self, _self);

  auto itr = inventory.find(sku);
  eosio_assert(itr != inventory.end(), 
                      "Product does not exist, use createitem() first");

  // eosio_assert(quantity > 0);

  inventory.modify(itr, store, [&](auto& item) {
      item.commonname = commonname;
      item.price = price;
  });

  eosio::print("Product updated: ", sku);
}

void ecobag::addstock(const account_name store, uint64_t sku, int32_t quantity) {
  eosio::require_auth(store);

  item_table inventory(_self, _self);

  auto itr = inventory.find(sku);
  eosio_assert(itr != inventory.end(), 
                      "Product does not exist, use createitem() first");

  int remains = static_cast<int32_t>(itr->quantity) + quantity;
  eosio_assert(remains >= 0, 
                      "Cannot reduce more than in stock");

  // eosio_assert(quantity > 0);

  inventory.modify(itr, store, [&](auto& item) {
      item.quantity = remains;
  });

  eosio::print("New item count: ", itr->quantity);

  if(itr->quantity == 0)
    eosio::print("remove item to minimize storage use");
}


void ecobag::removeitem(const account_name store, uint64_t sku) {
  eosio::require_auth(store);

  item_table inventory(_self, _self);

  auto itr = inventory.find(sku);
  eosio_assert(itr != inventory.end(), 
                      "Product does not exist");

  inventory.erase(itr);

  eosio::print("Product deleted: ", sku);
}

void ecobag::createcart(const account_name owner, const account_name store, const string& title) {
  eosio::require_auth(owner);

  eosio_assert(owner != store, 
                      "Needs to associate the store where items will come from");

  profile_table profiles(_self, _self);
  auto pstore = profiles.find(store);
  eosio_assert(pstore != profiles.end(), 
                      "Store not found");

  bag_table bags(_self, _self);
  auto bag = bags.find(owner);
  eosio_assert(bag == bags.end(), 
                      "User has existing cart, only one is allowed for now");

  bags.emplace(owner, [&](auto& bag) {
      bag.owner = owner;
      bag.store = store;
      bag.title = title;
      bag.status = 0;
      bag.orders = bag::itemlist{};
  });

  eosio::print("Created cart: ", title, "Start listing items to buy with addtobuy()");
}

void ecobag::updatecart(const account_name account, const string& title, uint16_t status) {
  eosio::require_auth(account);

  bag_table bags(_self, _self);

  auto itr = bags.find(account);
  eosio_assert(itr != bags.end(), 
                      "Cart not found");

  bags.modify(itr, account, [&](auto& bag) {
      bag.title = title;
      bag.status = status;
  });

  eosio::print("Updated cart: ", title);
}

void ecobag::addtocart(const account_name account, uint64_t sku, uint32_t quantity) {
  eosio::require_auth(account);

  bag_table bags(_self, _self);

  auto bag = bags.find(account);
  eosio_assert(bag != bags.end(), 
                      "Cart does not exist");

  item_table inventory(_self, _self);
  auto item = inventory.find(sku);
  eosio_assert(item != inventory.end(), 
                      "Item SKU not found");

  int remains = item->quantity - quantity;
  eosio_assert(remains >= 0, 
                      "Store has not enough stock of this item");

  bags.modify(bag, account, [&](auto& b) {
    bool found = false;
    for(auto& i : b.orders) {
      if(i.sku == sku) {
        found = true;
        i.count += quantity;
      }
    }
    if(!found) {
      b.orders.push_back({sku, quantity});
    }
  });

  eosio::print("Added to cart: ", sku);
}


void ecobag::clearcart(const account_name account) {
  eosio::require_auth(account);

  bag_table bags(_self, _self);

  auto bag = bags.find(account);
  eosio_assert(bag != bags.end(), 
                      "Cart does not exist");

  bags.erase(bag);

  eosio::print("Cart removed");
}

//void ecobag::checkoutcart(const account_name account) {
//}
