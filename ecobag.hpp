#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <string>

using std::string;

class ecobag : public eosio::contract {
public:
    explicit ecobag(account_name self) : eosio::contract(self) {}

    /////// account management actions

    // @abi action
    void createprofile(const account_name account, const string& username, const string& address);

    // @abi action
    void updateprofile(const account_name account, const string& username, const string& address);

    // @abi action
    void removeprofile(const account_name account);

    /////// store actions

    //
    // creates item and initializes count

    // @abi action
    void createitem(const account_name, uint64_t sku, const string& commonname, eosio::asset amount, uint32_t count);

    //
    // updates common name,
    // updates amount

    // @abi action
    void updateitem(const account_name, uint64_t sku, const string& commonname, eosio::asset amount);

    // 
    // adds or subtracts count

    // @abi action
    void addstock(const account_name, uint64_t sku, int32_t count);

    // 
    // removes the item from inventory

    // @abi action
    void removeitem(const account_name, uint64_t sku);

    /////// customer actions

    // @abi action
    void createcart(const account_name owner, const account_name store, const string& title);

    // @abi action
    void addtocart(const account_name owner, uint64_t sku, uint32_t count);

    // @abi action
    void clearcart(const account_name owner);

    //// @abi action
    void checkoutcart(const account_name owner, const account_name store);

    //// @abi action
    void readycart(const account_name owner, const account_name store);

    //// @abi action
    void pickup(const account_name owner, const account_name store, bool clear);

    //abi action
    //void xfer(const account_name toaccount, const account_name fromaccount, uint64_t sku, uint16_t count);

private:

  // @abi table item i64
  struct item
  {
      uint64_t sku;
      account_name store;
      string commonname;  // examples: milk, pasta, toothpaste, can be used
      eosio::asset amount;
      uint32_t count;

      // todo
      // string description; // could be a url
      // string brand;      
      // TPackage package;   // box of 10, 500mL pack, single piece
      // string summary; 
      // uint64_t expiry;
        
      auto primary_key() const { return sku; }
      account_name get_store() const { return store; }

      EOSLIB_SERIALIZE(item, (sku)(store)(commonname)(amount)(count))
  };
  typedef eosio::multi_index<N(item), item,
          eosio::indexed_by<N(bystore), eosio::const_mem_fun<item, account_name, &item::get_store>>>
          item_table;

  // @abi table profile i64
  struct profile
  {
      // account for store or customer
      account_name account;    // account_name is a typedef to uint64_t
      string username;
      string address;
      // todo
      // add active hours
      // add flag for pick-up/delivery
      // list of products

      auto primary_key() const { return account; }
      auto byusername() const { return username; }

      EOSLIB_SERIALIZE(profile, (account)(username)(address))
  };
  typedef eosio::multi_index<N(profile), profile>
          profile_table;

  // this is the cart, 
  // we call it a bag with list of orders
  // ownership will transfer to the store after list is updated (after multisig)
  // ownership will go back to the customer after pickup, 
  // so he can delete it or 
  // let contract auto-create it at a scheduled time (future versions)

  // @abi table bag i64
  struct bag
  {
      // warning: ABI not generated for enum types
      // use int to refer to OrderStatus
      enum OrderStatus {
          empty,
          updating_orders,
          waiting_for_store,
          received_by_store,
          ready_for_pickup,
          completed
      };

      uint64_t id;                // unused: needed when user can create more than 1 bag
      account_name owner;         
      account_name store;
      string title;               
      uint16_t status;            // int because ABI cannot generate enum
      eosio::asset total;
      uint64_t receiptId;         
      
      // this vector of struct will change once map or pair is supported in ABI generation
      // see issue https://github.com/EOSIO/eos/issues/354
      struct itempair
      {
        uint64_t sku;
        uint32_t count;
      };
      //typedef std::map<uint64_t, uint32_t> itemlist;
      typedef eosio::vector<itempair> itemlist;
      itemlist orders;

      // todo
      // account_name receiver;
      // uint64_t timeToPick;
      // uint64_t totalprice;
      auto primary_key() const { return owner; }
      //auto get_owner() const { return owner; }

      EOSLIB_SERIALIZE(bag, (id)(owner)(store)(title)(status)(orders)(receiptId))
  };
  typedef eosio::multi_index<N(bag), bag>
          bag_table;
  
  // this is the transaction record, amount will be taken from the cart data
  // the cart record cannot be reused because user may opt to delete it
  //
  // @abi table receipt i64
  struct receipt
  {
      uint64_t id;
      account_name from;
      account_name to;
      eosio::asset amount;
      std::string memo;

      uint64_t primary_key() const { return id; }
      EOSLIB_SERIALIZE(receipt, (id)(from)(to)(amount)(memo))
  };

  typedef eosio::multi_index<N(receipt), receipt>
          receipt_table;
};

EOSIO_ABI(ecobag, (createprofile)(updateprofile)(removeprofile)(createitem)(updateitem)(addstock)(removeitem)(createcart)(addtocart)(clearcart)(checkoutcart)(readycart)(pickup))

// notes 
// method names cannot be more than 13 characters, cannot contain big letters. { 1-5. a-z only }
// cannot generate ABI if using enum (only vector, struct, class, builtins can be generated)
// map/pair not supported by ABI, use vector of struct

// questions
// how to remove wallet lock timeout in keosd
// why transactions fail sometimes due to timeout deadline
// how to organize multiple smart contracts in a big project, ie.. follow single responsibility
// how to restart nodeos and resync blocks: error producer_plugin.cpp:627 Not producing block because the irreversible block is too old [age:123584s, max:1800s]
