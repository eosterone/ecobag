# ecobag
Smart contracts to facilitate shopping and delivery of goods. This is my first attempt at creating dapp based on EOS.


# Version 1 
Will be for a simple store-pickup/delivery scenario. 
    Buyer selects items online, fills up the virtual cart/bag, assigns himself or someone else to receive/pick it up.  
    Store fills the bag and readies it for pickup/deliver,  
    Authorized person picks up the actual bag, and confirms receipt.  
    Contract finalizes transaction, taking payment from Buyer.  
  
    store actions:  
        creates store account with name, address, operating hours  
        maintains a list of available products  
  
    buyer actions:  
        creates buyer account with name  
        selects items to buy from a single store (version 1),  
        closes the virtual bag,  
        confirms transaction after picking up the actual bag(s)  
  
    ecobag contract:  
        bag has fields for (from, to, items and price, deadline, ready flag)  
        buyer creates bag {uid, name of store, name of owner, name of receiver, time of pickup}  
        buyer adds/deletes items to/from the bag, contract verifies if buyer has available balance  
        store marks the bag as ready for pickup  
        buyer picks up and confirms  
        contract does the token transactions  
        bag is deleted  
  
  
# Actions and Sample Usage (WIP)
see test_scenario.sh for sample usage

# Profile Management  
profile does not distinguish between store and customer, so that the actions are not tightly dependent on the account.  
  
related actions:  
void createprofile(const account_name account, const string& username, const string& address);  
void updateprofile(const account_name account, const string& username, const string& address);  
void removeprofile(const account_name account);  
  
# Inventory
adds item to the inventory, keyed by sku and store_account. Items are added by store accounts, and accessed by customers when updating the to-buy list. The to-buy list is a vector of skus found in the inventory table.
  
related actions:  
void createitem(const account_name, uint64_t sku, const string& commonname, const eosio::asset& amount, int64_t count);  
void updateitem(const account_name, uint64_t sku, const string& commonname, const eosio::asset& amount);  
void addstock(const account_name, uint64_t sku, int64_t count);  
void removeitem(const account_name, uint64_t sku);  
    
# Cart, Pickup or Delivery, and Transactions
customer creates the cart which will contain the list of items he wants to buy from the store. It is assumed for now that all items come from the same associated store. The authorized account who will receive/pick up the items is also listed (it can be a different account from the owner). When the customer marks the cart as ready for checkout, the store account takes ownership and prepares the physical items. Upon pickup, both accounts push an action that will trigger the transactions.  
  
related actions:  
void createcart(const account_name owner, const account_name store, const string& title);  
void addtocart(const account_name owner, uint64_t sku, int64_t count);  
void clearcart(const account_name owner);  
void checkoutcart(const account_name owner, const account_name store);  
void readycart(const account_name owner, const account_name store);  
void pickup(const account_name owner, const account_name store, bool clear);  
    
# Version 2  
Version 2 will minimize the participation of stores by introducing "personal shopper" and/or "courier"  
        - allows buyer to make multiple bags from different stores  
        - allows personal shopper to do the actual shopping and delivery (charges a fee)  
        
        
 
