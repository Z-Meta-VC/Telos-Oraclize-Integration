// Todd Fleming 2018

#include <eosiolib/eosio.hpp>
#include <eosiolib/action.hpp>
#include <eosiolib/currency.hpp>
#include <eosiolib/multi_index.hpp>

using eosio::action;
using eosio::asset;
using eosio::currency;
using eosio::name;
using eosio::permission_level;
using eosio::print;
using eosio::require_auth;
using eosio::string_to_symbol;
using eosio::symbol_type;
using eosio::unpack_action_data;

struct balance {
    asset funds;
    uint64_t primary_key() const { return funds.symbol; }
};
using table = eosio::multi_index<N(balance), balance>;

struct withdraw {
    uint64_t user;
};

void transferAction(uint64_t self, uint64_t code) {
    eosio_assert(code == N(eosio.token), "I reject your non-eosio.token deposit");
    auto data = unpack_action_data<currency::transfer>();
    if(data.from == self || data.to != self)
        return;
    eosio_assert(data.quantity.symbol == string_to_symbol(4, "EOS"), 
        "I think you're looking for another contract");
    eosio_assert(data.quantity.is_valid(), "Are you trying to corrupt me?");
    eosio_assert(data.quantity.amount > 0, "When pigs fly");

    table balances(self, data.from);
    asset new_balance;
    auto it = balances.find(data.quantity.symbol);
    if(it != balances.end())
        balances.modify(it, data.from, [&](auto& bal){
            // Assumption: total currency issued by eosio.token will not overflow asset
            bal.funds += data.quantity;
            new_balance = bal.funds;
        });
    else
        balances.emplace(data.from, [&](auto& bal){
            bal.funds = data.quantity;
            new_balance = bal.funds;
        });

    print(name{data.from}, " deposited:       ", data.quantity, "\n");
    print(name{data.from}, " funds available: ", new_balance);
}

void withdrawAction(uint64_t self, uint64_t code) {
    if (code != self)
        return;
    auto data = unpack_action_data<withdraw>();
    require_auth(data.user);

    table balances(self, data.user);
    auto it = balances.find(string_to_symbol(4, "EOS"));
    if(it == balances.end()) {
        print(name{data.user}, " has no funds to withdraw.\n");
        print("no transfer needed.\n");
        return;
    }

    print(name{data.user}, " has ", it->funds, " available\n");
    print("returning everything");

    action{
        permission_level{self, N(active)},
        N(eosio.token),
        N(transfer),
        currency::transfer{
            .from=self, .to=data.user, .quantity=it->funds, .memo=""}
    }.send();
    balances.erase(it);
}

extern "C" void apply(uint64_t receiver, uint64_t code, uint64_t action) {
    switch(action) {
        case N(transfer): return transferAction(receiver, code);
        case N(withdraw): return withdrawAction(receiver, code);
    }
}

//via todd flemming, thank you.
