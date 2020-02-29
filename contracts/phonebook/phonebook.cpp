#include <eosiolib/eosio.hpp>
#include <eosiolib/action.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/print.hpp>
#include <string>

using std::string;
using eosio::const_mem_fun;
using eosio::indexed_by;

struct hi {
    account_name account;
};

class phonebook : public eosio::contract {
   public:
      explicit phonebook(action_name self)
              : contract(self) {
      }

      //@abi action
      void add(const account_name account,
               const string& phone,
               const string& first_name,
	       const string& last_name,
               const string& street,
	       const string& city,
	       const string& state) {


          // premission must be granted for this action, if not authorized then this action is aborted and transaction is rolled back
          require_auth(account); 

          // record_index is typedef of eosio multi_index over table record
          record_index records(_self, _self); // code, scope

          // verify phone record does not already exist
          // multi_index find on primary index which is account
          auto itr = records.find(account);
	  
          eosio_assert(itr == records.end(), "record for account already exists");

          // if not exist - reches this code and insert another record 
          records.emplace(account, [&](auto& record) {
              record.account_name = account;
              record.phone_number = phone;
              record.first_name = first_name;
              record.last_name = last_name;
              record.street = street;
              record.city = city;
              record.state = state;
          });

          eosio::action(
                  std::vector<eosio::permission_level>(1,{_self, N(active)}),
                  N(hello), N(hi), hi{account} ).send();
      }

      //@abi action
      void update(const account_name account,
                const string& phone,
                const string& first_name, 
  		const string& last_name,
		const string& street,
		const string& city,
 		const string& state) {

          require_auth(account); 

          record_index records(_self, _self); // code, scope

          // verify account record already exist
          auto itr = records.find(account);
          eosio_assert(itr != records.end(), "record for account not found");

          // if exist - reches this code and updates record
          records.modify( itr, account , [&]( auto& record ) {
              record.account_name = account;
              record.phone_number = phone;
              record.first_name = first_name;
              record.last_name = last_name;
              record.street = street;
              record.city = city;
              record.state = state;
          });
      }

      //@abi action
      void remove(const account_name account) {
          require_auth(account); 

          record_index records(_self, _self); // code, scope

          // verify record already exist
          auto itr = records.find(account);
          eosio_assert(itr != records.end(), "Record for account not found");

          records.erase( itr );
      }

  //@abi action
      void approve(const account_name account) {
          // do not require_auth since we want to allow anyone to call

          record_index records(_self, _self); // code, scope

          // verify account record already exist
          auto itr = records.find(account);
          eosio_assert(itr != records.end(), "record for account not found");

          records.modify( itr, 0 , [&]( auto& record ) {
              eosio::print("approving: ", record.phone_number.c_str()," Is " ,
                           record.first_name.c_str(), " ", record.last_name.c_str(),  " Correct Phone Number\n");
              record.approves++;
          });
      }


   private:

      //@abi table record i64
      struct record {
          uint64_t account_name;
          string phone_number;
          string first_name;
          string last_name;
          string street;
          string city;
          string state;
          uint64_t approves = 0;

          uint64_t primary_key() const { return account_name; }
          uint64_t by_approves() const { return approves; }

          EOSLIB_SERIALIZE( record, (account_name)(first_name)(last_name)(street)(city)(state)(phone_number)(approves) )
      };

      typedef eosio::multi_index< N(record), record,
         indexed_by< N(approves), const_mem_fun<record, uint64_t, &record::by_approves> >
      > record_index;

};

EOSIO_ABI( phonebook, (add)(update)(remove)(approve) )



