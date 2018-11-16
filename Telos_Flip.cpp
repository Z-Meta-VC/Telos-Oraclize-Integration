#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>

#include "eos_api.hpp"

using namespace eosio;


class flipper : public eosio::contract {
  public:
      using contract::contract;
      int choice;

      /// @abi action
      void getflip(int _choice) {
         //TODO: accept payment
         choice = _choice;
         oraclize_query(10, "WolframAlpha", "random number between 0 and 1");
         print("Oraclize query was sent, standing by for the answer..");
      }

      /// @abi action
      void callback( checksum256 queryId, std::vector<unsigned char> result, std::vector<unsigned char> proof ) {
         require_auth(oraclize_cbAddress());

         std::string result_str = vector_to_string(result);
         print("Result:", result_str);

         if (result_str == choice){
            //you win
            
            //TODO send money back plus a prize - a small fee
         }else{
            //you lose
            //We keep your money
         }
      }

};

EOSIO_ABI(flipper , (getflip)(callback))
