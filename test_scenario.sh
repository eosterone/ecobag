#!/bin/bash

# must be inside eos/build directory
# make sure nodeos is accessible and wallet is unlocked

#------------------------------------------------
# prepare test accounts and token

# customer
cleos wallet import 5KV8tMzVzfB824H4on2UeeJXLBGbtqCBkgxzM5ZvPHPZzkUvYiu 
cleos wallet import 5JHXHGNjHnL67mNaZEzWu7W72Rc4ZM9EHfWrZHhfrkZs9tae13k 
cleos create account eosio eosterone EOS744BQfCMAsxVCrURSD7eQc1bRuFoJTossxnhs4xx6yDwUop6f7 EOS565tRNrJQe3uLs79yVoX9NJjNrqm6gtpWWx7S655viCP72vgtU

# store
cleos wallet import 5K2MDD3xLGCj3SxSLLQfLbRJSUz3rNLiRfw3k4hHDCrtQzhisXV 
cleos wallet import 5JNW9WgVY7KrGC6z1MuzsQ5nuWFcsEUgbYNW46QTMX6BD4HLntQ
cleos create account eosio grocery EOS63gpZKJM9gF4jKf8MJFxjeo5SRZiDeg7ZxrARLBqbm5JsBhazt EOS5i1tyhaj6C3PRUbzQcXs82UZQ8yjqbvzNXAzpwuRB1NFEjVv9g

# optional receiver
cleos wallet import 5JwHKp1ruHf52TStsdSFF9Lgr5zuZEhvJke4oTVFKYNvUTUYbVh
cleos wallet import 5JosYAtLGsmUNjgsRVTgZF6oRLpDRsrJGNPze14YQR9yrpJW8cz
cleos create account eosio lazyfriend EOS8Ahtd3sB1CvjBgZDufbmXaNfnBKQtaxv9Me2ZCLaWfhVLK9viZ EOS6HjvWwpK2iUxZPsnXWTpqdR3R6MUPnRaWFz3U4Ghs3Mg8t4Xn5

#------------------------------------------------
# issue SYS tokens and transfer to test account

cleos wallet import 5JseSEsoVwBH6WRsx5CWYWAe6pNG9tpSc5ARynxCMzUV3X6T7GA
cleos wallet import 5KKNYgqYmzswcSzi26NFhvcNm6SrogAqCqQv6ZDZWbKoFtnyLBe
cleos create account eosio token EOS6uLiaG26JQYaEacTkGdWcZVtpRZgvEzEhfvEkwXUs4x6uQekMW EOS6e5wudqwZZnQLXHr9vnjfRLMBajKjDuuA8XLJGqfxhA4hrmg2T
cleos set contract token ./contracts/eosio.token/ ./contracts/eosio.token/eosio.token.wast ./contracts/eosio.token/eosio.token.abi
cleos push action token create '{"issuer":"token","maximum_supply":"1000000.0000 SYS","can_freeze":"0","can_recall":"0","can_whitelist":"0"}' -p token
cleos push action token issue '{"to":"token","quantity":"1000.0000 SYS","memo":""}' -p token
cleos get table token token accounts
cleos push action token transfer '{"from":"token","to":"eosterone","quantity":"200.0000 SYS","memo":"for testing"}' -p token
cleos get table token eosterone accounts

#------------------------------------------------
# set ecobag account and publish contract to node

cleos wallet import 5KHUB1fQJoupX8WMeEvBPdfcSrGTyvZrrs57fq9ytzWuprmK2CL
cleos wallet import 5KWEK2UE6PNiYbMKdL9hc4ZwX9V8zbX18qhJGMWjxo7iSeEmpJv
cleos create account eosio ecobag EOS5Ex63S71YTPbynpJXngUmiev6jvdUizgK7c3T86yNroaNUNxFn EOS5LPiKGcmJJMNzppq4GtB8km8dio3b3D23G1ZEvCQrtwYz8CZWM
eosiocpp -o ../contracts/ecobag/ecobag.wast ../contracts/ecobag/ecobag.cpp &&
eosiocpp -g ../contracts/ecobag/ecobag.abi ../contracts/ecobag/ecobag.hpp &&
cleos set contract ecobag ../contracts/ecobag/ ../contracts/ecobag/ecobag.wast ../contracts/ecobag/ecobag.abi

#------------------------------------------------
# create test profiles

cleos push action ecobag createprofile '["eosterone", "EOSTERONE", "manila"]' -p eosterone
cleos push action ecobag createprofile '["grocery", "GROCERY", "makati"]' -p grocery
cleos push action ecobag createprofile '["lazyfriend", "LAZYFRIEND", "cebu"]' -p lazyfriend
cleos get table ecobag ecobag profile

#------------------------------------------------
# add items to inventory

cleos push action ecobag createitem '["grocery", "1111", "beercan", "10 SYS", "100"]' -p grocery
cleos push action ecobag addstock '["grocery", "1111", "-1"]' -p grocery
cleos push action ecobag createitem '["grocery", "1112", "infantformula", "55 SYS", "25"]' -p grocery
cleos push action ecobag updateitem '["grocery", "1112", "infantmilk", "56 SYS"]' -p grocery
cleos get table ecobag ecobag item

#------------------------------------------------
# take orders 

cleos push action ecobag createcart '["eosterone", "grocery", "weeklygroceries"]' -p eosterone
cleos push action ecobag addtocart '["eosterone", "1111", "8"]' -p eosterone
cleos push action ecobag addtocart '["eosterone", "1112", "2"]' -p eosterone
cleos get table ecobag ecobag bag

#------------------------------------------------
# checkout and do transaction 

cleos push action ecobag checkoutcart '["eosterone", "grocery"] - eosterone -p grocery
cleos push action ecobag pickup '["eosterone", "grocery"] - eosterone -p grocery
cleos get table ecobag ecobag bag

#------------------------------------------------
#check transaction record

cleos get table ecobag ecobag receipt
