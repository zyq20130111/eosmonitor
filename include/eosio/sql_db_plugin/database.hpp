#pragma once
#include <soci/soci.h>

#include <eosio/chain/config.hpp>
#include <eosio/chain/block_state.hpp>
#include <eosio/chain/transaction.hpp>
#include <eosio/chain/trace.hpp>
#include <eosio/chain/types.hpp>

#include <eosio/sql_db_plugin/accounts_table.hpp>
#include <eosio/sql_db_plugin/transactions_table.hpp>
#include <eosio/sql_db_plugin/blocks_table.hpp>
#include <eosio/sql_db_plugin/actions_table.hpp>
#include <eosio/sql_db_plugin/session_pool.hpp>

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

namespace eosio {

struct trace_and_block_time{
    chain::transaction_trace_ptr trace;
    chain::block_timestamp_type block_time;
};

class sql_database {
    public:
        sql_database(const std::string& uri, uint32_t block_num_start, size_t pool_size);
        sql_database(const std::string& uri, uint32_t block_num_start, size_t pool_size, std::vector<std::string>, std::vector<std::string>);
        
        void wipe();
        bool is_started();
        void consume_block_state( const chain::block_state_ptr& );
        void consume_irreversible_block_state( const chain::block_state_ptr& , boost::mutex::scoped_lock& , boost::condition_variable& condition,boost::atomic<bool>& exit);

        void consume_transaction_metadata( const chain::transaction_metadata_ptr& );
        void consume_transaction_trace( const trace_and_block_time& );

        void dfs_inline_traces( std::shared_ptr<soci::session>, vector<chain::action_trace>,  chain::transaction_id_type, chain::block_timestamp_type );
        int get_min_account_id();
        int get_max_account_id();
        bool monitoraccount(int accountid);

        bool update_token(std::string account);
        void save_token(std::string account,std::string symbol,std::string quantity,int precision,std::string contract);
        
        bool update_stake(std::string account);
        void save_stake(
                std::string account,
                int64_t liquid,
                int64_t staked,
                int64_t unstaking,
                int64_t total,
                int64_t total_stake,
                int64_t totalasset,
                int64_t cpu_total,
                int64_t cpu_staked,
                int64_t cpu_delegated,
                int64_t cpu_used,
                int64_t cpu_available,
                int64_t cpu_limit,
                int64_t net_total,
                int64_t net_staked,
                int64_t net_delegated,
                int64_t net_used,
                int64_t net_available,
                int64_t net_limit,
                int64_t ram_quota,
                int64_t ram_usage
                );

        std::shared_ptr<soci_session_pool> m_session_pool;
        std::unique_ptr<actions_table> m_actions_table;
        std::unique_ptr<accounts_table> m_accounts_table;
        std::unique_ptr<blocks_table> m_blocks_table;
        std::unique_ptr<transactions_table> m_transactions_table;
        std::string system_account;
        uint32_t m_block_num_start;
        std::vector<std::string> m_action_filter_on;
        std::vector<std::string> m_contract_filter_out;

    };

} // namespace

