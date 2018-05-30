# ecobag
Smart contracts to facilitate shopping and delivery of goods. This is my first attempt at creating dapp based on EOS.


# Version 1 
Will be for a simple store-pickup/delivery scenario. 
    Buyer selects items online, fills up the virtual cart/bag, assigns himself or someone else to receive/pick it up.  
    Store fills the bag and readies it for pickup/deliver,  
    Buyer/Authorized person picks up the actual bag, and confirms receipt.  
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
  
# Version 2  
Version 2 will minimize the participation of stores by introducing "personal shopper" and/or "courier"  
        - allows buyer to make multiple bags from different stores  
        - allows personal shopper to do the actual shopping and delivery (charges a fee)  
