#!/bin/bash

nodeos -e -p eosio --plugin eosio::wallet_api_plugin --plugin eosio::chain_api_plugin --plugin eosio::history_api_plugin --max-irreversible-block-age 168000 --contracts-console --hard-replay