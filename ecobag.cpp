#include <ecobag.hpp>
#include <eosiolib/print.hpp>

using namespace eosio;

void ecobag::createprofile(const account_name account, const string& username, const string& address) {
    
  eosio::require_auth(account); // optional: , N(active)

  // Why define it everytime, cant this be a member?
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

void ecobag::createitem(const account_name store, uint64_t sku, const string& commonname, const eosio::asset& amount, int64_t count) {
  eosio::require_auth(store);

  item_table inventory(_self, _self);

  auto itr = inventory.find(sku);
  eosio_assert(itr == inventory.end(), 
                      "Product SKU already exists, use addproduct() to update count");

  // eosio_assert(count > 0);
  inventory.emplace(store, [&](auto& item) {
      item.sku = sku;
      item.store = store;
      item.commonname = commonname;
      item.amount = amount;
      item.count = count;
  });

  eosio::print("Product created: ", sku);
}

void ecobag::updateitem(const account_name store, uint64_t sku, const string& commonname, const eosio::asset& amount) {
  eosio::require_auth(store);

  item_table inventory(_self, _self);

  auto itr = inventory.find(sku);
  eosio_assert(itr != inventory.end(), 
                      "Product does not exist, use createitem() first");

  // eosio_assert(count > 0);

  inventory.modify(itr, store, [&](auto& item) {
      item.commonname = commonname;
      item.amount = amount;
  });

  eosio::print("Product updated: ", sku);
}

void ecobag::addstock(const account_name store, uint64_t sku, int64_t count) {
  eosio::require_auth(store);

  item_table inventory(_self, _self);

  auto itr = inventory.find(sku);
  eosio_assert(itr != inventory.end(), 
                      "Product does not exist, use createitem() first");

  int64_t remains = itr->count + count;
  eosio_assert(remains >= 0, 
                      "Cannot reduce more than in stock");

  // eosio_assert(count > 0);

  inventory.modify(itr, store, [&](auto& item) {
      item.count = remains;
  });

  eosio::print("New item count: ", itr->count);

  if(itr->count == 0)
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
      bag.id = bags.available_primary_key();
      bag.owner = owner;
      bag.store = store;
      bag.title = title;
      bag.status = bag::OrderStatus::empty;
      bag.orders = bag::itemlist{};
  });

  eosio::print("Created cart: ", title, "Start listing items to buy with addtobuy()");
}

void ecobag::addtocart(const account_name owner, uint64_t sku, int64_t count) {
  eosio::require_auth(owner);

  bag_table bags(_self, _self);

  auto bag = bags.find(owner);
  eosio_assert(bag != bags.end(), "Cart does not exist");
  eosio_assert( (bag->status == bag::OrderStatus::empty) ||
                (bag->status == bag::OrderStatus::updating_orders),
                "Orders are being processed. Cannot update list");
  
  item_table inventory(_self, _self);
  auto item = inventory.find(sku);
  eosio_assert(item != inventory.end(), 
                      "Item SKU not found");

  int64_t remains = item->count - count;
  eosio_assert(remains >= 0, 
                      "Store has not enough stock of this item");

  eosio::print("addtocart: passed all assertions..");

  //asset thisAmount = item->amount * count;
  // todo: assert owner's balance
  asset thisAmount{item->amount.amount * count, S(4, SYS)};
  eosio::print("thisamount: ", thisAmount);

  bags.modify(bag, owner, [&](auto& b) {
    bool found = false;
    for(auto& order : b.orders) {
      if(order.sku == sku) {
        eosio::print("mod item count");
        found = true;
        order.count += count;
      }
    }
    if(!found) {
      eosio::print("pushback new itempair");
      b.orders.push_back({sku, count});
    }

    b.status = bag::OrderStatus::updating_orders;
    b.total += thisAmount;
  });

  eosio::print("Added to cart: ", sku);
}


void ecobag::clearcart(const account_name owner) {
  eosio::require_auth(owner);

  bag_table bags(_self, _self);
  auto bag = bags.find(owner);
  eosio_assert(bag != bags.end(), 
                      "Cart does not exist");
  //eosio_assert( bag->status != bag::OrderStatus::received_by_store &&
  //              bag->status != bag::OrderStatus::ready_for_pickup &&
  //              "Orders are being processed. Cannot update list");

  bags.erase(bag);

  eosio::print("Cart removed");
}

void ecobag::checkoutcart(const account_name owner, const account_name store) {
  eosio::require_auth(owner);
  eosio::require_auth(store);

  bag_table bags(_self, _self);
  auto bag = bags.find(owner);
  eosio_assert(bag != bags.end(), "Cart does not exist");

  eosio_assert(bag->store == store, "This order is to be fullfilled by another store");
  //eosio_assert(bag->status == bag::OrderStatus::waiting_for_store, "Orders are not yet final");

  item_table items(_self, _self);

  int64_t totalAmount = 0;
  bool verified = true;
  for(const auto& order : bag->orders) {

    auto item = items.find(order.sku);
    if(item == items.end()) {
      eosio::print("item not found: ", order.sku);
      verified = false;
      break;  
    }

    if(item->count < order.count) {
      eosio::print("not enough stock: ", item->count);
      verified = false;
      break; 
    }

    totalAmount += (item->amount.amount * order.count);
  }

  eosio_assert(verified, "Not all items are in the store inventory");

  //asset thisAmount{totalAmount, S(4, SYS)};
  eosio_assert(totalAmount == bag->total.amount, "Total amount dont match");
  // todo verify account balance
  // ...

  // storage consumption charged to store

  bags.modify(bag, store, [&](auto& b) {
      b.status = bag::OrderStatus::received_by_store;
  });
  
  eosio::print("Store is preparing items");
}

void ecobag::readycart(const account_name store, const account_name owner) {
  eosio::require_auth(store);

  bag_table bags(_self, _self);
  auto bag = bags.find(owner);
  eosio_assert(bag != bags.end(), "Cart does not exist");
  eosio_assert(bag->store == store, "Cart not assigned to store");
  eosio_assert(bag->status == bag::OrderStatus::received_by_store, "Store has not received the orders");

  // update store inventory
  item_table inventory(_self, _self);
  for(const auto& order : bag->orders) {
    auto item = inventory.find(order.sku);
    if(item != inventory.end()) {
      inventory.modify(item, store, [&](auto& item) {
        if(order.count <= item.count)
          item.count -= order.count;
        else
          item.count = 0; // or assert?
      });
    }
  }

  // storage consumption charged to store
  bags.modify(bag, store, [&](auto& b) {
      b.status = bag::OrderStatus::ready_for_pickup;
  });
}

void ecobag::pickup(const account_name owner, const account_name store, bool clear) {
  eosio::require_auth(owner);
  eosio::require_auth(store);

  bag_table bags(_self, _self);
  auto bag = bags.find(owner);

  eosio_assert(bag->status == bag::OrderStatus::ready_for_pickup, "Orders not ready");
  eosio_assert(bag != bags.end(), "Cart does not exist");
 
  // todo: do transaction
  // transfer amount (bag->total) from account to store

  eosio::action( std::vector<eosio::permission_level> (1, {owner, N(active)}),
    N(token), N(transfer), tokentrans{owner, store, bag->total, "some memo"}).send();
  // getting this error here
  // Error 3090003: provided keys, permissions, and delays do not satisfy declared authorizations
  // does not have signatures for it under a provided delay of 0 ms

  // create empty receipt, id will be assigned to the cart
  receipt_table receipts(_self, _self);

  receipts.emplace(store, [&](auto& rec) {
    rec.id = receipts.available_primary_key();
    rec.from = owner;
    rec.to = store;
    rec.amount = bag->total;
    rec.memo = "some_memo";
  });

  // forget history or set cart to be reused again
  if(clear) {
    bags.erase(bag);
  }
  else {
    bags.modify(bag, owner, [&] (auto& b) {
      b.status = bag::OrderStatus::completed;
    });
  }

  eosio::print("Transaction Complete");
}

// @abi action
void ecobag::reactivate(const account_name owner) {
  eosio::require_auth(owner);

  bag_table bags(_self, _self);
  auto bag = bags.find(owner);
  eosio_assert(bag != bags.end(), "Cart not found. Create one with createcart()");
  eosio_assert(bag->status == bag::OrderStatus::completed, "Cart is active, nothing to do here.");

  bags.modify(bag, owner, [&](auto& b) {
      b.status = bag::OrderStatus::waiting_for_store;
  });

  eosio::print("Reactivated cart: ", bag->title);
}