/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

#pragma once

#include <eosio/chain_plugin/chain_plugin.hpp>
#include <eosio/chain/contract_table_objects.hpp>
#include <eosio/sql_db_plugin/database.hpp>
#include <appbase/application.hpp>
#include <boost/signals2/connection.hpp>
#include <memory>


namespace eosio {

    using namespace chain;
    using std::shared_ptr;

    typedef shared_ptr<class sql_db_plugin_impl> sql_db_ptr;
    typedef shared_ptr<const class sql_db_plugin_impl> sql_db_const_ptr;

namespace sql_db_apis{

class read_only{

    public:

        const controller& db;
        const fc::microseconds abi_serializer_max_time;
        const std::shared_ptr<sql_database> sql_db;

        read_only(const controller& db, const fc::microseconds& abi_serializer_max_time, const std::shared_ptr<sql_database> sql_db)
            : db(db), abi_serializer_max_time(abi_serializer_max_time),sql_db(sql_db) {
                int a = 0;
                a = a + 1;
            }

        //get tokens
        struct token {
            account_name     contract;
            string           quantity;
            string           symbol;
            uint8_t          precision;
        };

        struct token_params{
            name             contract;
            string           symbol;
            uint8_t          precision = 255;
        };

        struct get_tokens_params{
            account_name         account;
            vector<token_params> tokens;
        };

        struct get_tokens_result{
            vector<token> tokens;
        };

        get_tokens_result get_tokens( const get_tokens_params& p )const;

        //get all tokens
        struct get_all_tokens_params{
            account_name account;
            int startNum = 0;
            int pageSize = 10;
        };

        struct get_all_tokens_result{
            vector<token> tokens;
        };

        get_all_tokens_result get_all_tokens( const get_all_tokens_params& p )const;

        struct get_hold_tokens_params{
            account_name account;
        };

        struct get_hold_tokens_result{
            vector<token> tokens;
        };

        get_hold_tokens_result get_hold_tokens( const get_hold_tokens_params& p )const;

        //get user resource
        struct get_userresource_params{
            account_name account;
        };

        struct get_userresource_result{
            asset    net_weight;
            asset    cpu_weight;
            int64_t  ram_bytes = 0;
        };

        get_userresource_result get_userresource( const get_userresource_params& p )const;

        //get stake
        // struct get_stake_params{
        //     account_name account;
        // }

        // struct get_stake_result{
        //     asset net_stake_for_self;
        //     asset cpu_stake_for_selt;
        //     asset net_stake_for_other;
        //     asset cpu_stake_for_other;
        // }

        // get_stake_result get_stake( const get_stake_params& p )const;
        
        //get refund
        struct get_refund_params{
            account_name account;
        };

        struct get_refund_result{
            string request_time;
            asset net_amount;
            asset cpu_amount;
        };

        get_refund_result get_refund( const get_refund_params& p )const;

        //multisig
        struct get_multisig_params{
            account_name account;
        };

        struct proposal{
            name proposer;
            name proposal_name;
            string transaction;
            string requested_approvals;
            string provided_approvals;
        };

        struct get_multisig_result{
            vector<proposal> proposal;
        };

        get_multisig_result get_multisig( const get_multisig_params& p )const;

        // search info
        template<typename Function, typename Function2>
        void walk_key_value_table(const name& code, const name& scope, const name& table, Function f, Function2 f2) const;

        string asset_amount_to_string(asset cursor) const{
            string sign = cursor.get_amount() < 0 ? "-" : "";
            int64_t abs_amount = std::abs(cursor.get_amount());
            string result = fc::to_string( static_cast<int64_t>(abs_amount) / cursor.precision());
            if( cursor.decimals() )
            {
                auto fract = static_cast<int64_t>(abs_amount) % cursor.precision();
                result += "." + fc::to_string(cursor.precision() + fract).erase(0,1);
            }
            return sign + result;
        }

};

} // namespace sql_db_apis

/**
 * @author Alessandro Siniscalchi <asiniscalchi@gmail.com>
 *
 * Provides persistence to SQL DB for:
 *   Blocks
 *   Transactions
 *   Actions
 *   Accounts
 *
 *   See data dictionary (DB Schema Definition - EOS API) for description of SQL DB schema.
 *
 *   The goal ultimately is for all chainbase data to be mirrored in SQL DB via a delayed node processing
 *   blocks. Currently, only Blocks, Transactions, Messages, and Account balance it mirrored.
 *   Chainbase is being rewritten to be multi-threaded. Once chainbase is stable, integration directly with
 *   a mirror database approach can be followed removing the need for the direct processing of Blocks employed
 *   with this implementation.
 *
 *   If SQL DB env not available (#ifndef SQL DB) this plugin is a no-op.
 */

class sql_db_plugin : public plugin<sql_db_plugin> {
    public:
        APPBASE_PLUGIN_REQUIRES((chain_plugin))
        sql_db_apis::read_only  get_read_only_api()const;

        sql_db_plugin();
        virtual ~sql_db_plugin();

        virtual void set_program_options(options_description& cli, options_description& cfg) override;
        virtual void plugin_initialize(const variables_map& options);
        virtual void plugin_startup();
        virtual void plugin_shutdown();

    private:
        sql_db_ptr my;
    
};

}

FC_REFLECT(eosio::sql_db_apis::read_only::token_params, (contract)(symbol)(precision) )
FC_REFLECT(eosio::sql_db_apis::read_only::get_tokens_params, (account)(tokens) )
FC_REFLECT(eosio::sql_db_apis::read_only::get_tokens_result, (tokens) )

FC_REFLECT(eosio::sql_db_apis::read_only::token, (contract)(quantity)(symbol)(precision) )
FC_REFLECT(eosio::sql_db_apis::read_only::get_all_tokens_params, (account)(startNum)(pageSize) )
FC_REFLECT(eosio::sql_db_apis::read_only::get_all_tokens_result, (tokens) )

FC_REFLECT(eosio::sql_db_apis::read_only::get_hold_tokens_params, (account))
FC_REFLECT(eosio::sql_db_apis::read_only::get_hold_tokens_result, (tokens) )

FC_REFLECT(eosio::sql_db_apis::read_only::get_userresource_params, (account) )
FC_REFLECT(eosio::sql_db_apis::read_only::get_userresource_result, (net_weight)(cpu_weight)(ram_bytes) )

FC_REFLECT(eosio::sql_db_apis::read_only::get_refund_params, (account) )
FC_REFLECT(eosio::sql_db_apis::read_only::get_refund_result, (request_time)(net_amount)(cpu_amount) )

FC_REFLECT(eosio::sql_db_apis::read_only::get_multisig_params, (account) )
FC_REFLECT(eosio::sql_db_apis::read_only::proposal, (proposer)(proposal_name)(transaction)(requested_approvals)(provided_approvals) )
FC_REFLECT(eosio::sql_db_apis::read_only::get_multisig_result, (proposal) )




