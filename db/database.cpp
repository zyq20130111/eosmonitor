// #include "database.hpp"
#include <eosio/sql_db_plugin/database.hpp>
#include <eosio/sql_db_plugin/sql_db_plugin.hpp>
#include <eosio/chain_plugin/chain_plugin.hpp>


namespace eosio
{

    sql_database::sql_database(const std::string &uri, uint32_t block_num_start, size_t pool_size) {
        m_session_pool          = std::make_shared<soci_session_pool>(pool_size,uri);
        m_accounts_table        = std::make_unique<accounts_table>();
        m_blocks_table          = std::make_unique<blocks_table>();
        m_transactions_table    = std::make_unique<transactions_table>();
        m_actions_table         = std::make_unique<actions_table>();
        m_block_num_start       = block_num_start;
        system_account          = chain::name(chain::config::system_account_name).to_string();
    }

    sql_database::sql_database(const std::string &uri, uint32_t block_num_start, size_t pool_size, std::vector<string> filter_on, std::vector<string> filter_out) {
        new (this)sql_database( uri, block_num_start, pool_size );
        m_action_filter_on = filter_on;
        m_contract_filter_out = filter_out;
    }

    void sql_database::wipe() {
        chain::abi_def abi_def;
        abi_def = eosio_contract_abi(abi_def);
        m_accounts_table->add_eosio( m_session_pool->get_session(), system_account, fc::json::to_string( abi_def ));
    }

    bool sql_database::is_started() {
        return m_accounts_table->exist( m_session_pool->get_session(), system_account );
    }

    void sql_database::consume_block_state( const chain::block_state_ptr& bs) {
        // auto m_session = m_session_pool->get_session();


        auto block_id = bs->id.str();

        if(this->m_blocks_table != nullptr) 
               m_blocks_table->add(m_session_pool->get_session(),bs);

        for(auto& receipt : bs->block->transactions) {
            string trx_id_str;
            if( receipt.trx.contains<chain::packed_transaction>() ){
                const auto& trx = fc::raw::unpack<chain::transaction>( receipt.trx.get<chain::packed_transaction>().get_raw_transaction() );
                
                m_transactions_table->add(m_session_pool->get_session(),trx);

                if(trx.actions.size()==1 && trx.actions[0].name.to_string() == "onblock" ) continue ;

                for(const auto& act : trx.actions){
                    m_actions_table->add( m_session_pool->get_session(), act,trx.id(), bs->block->timestamp, m_action_filter_on);
                }

            }         

        }
    }

    void sql_database::consume_transaction_trace( const trace_and_block_time& tbt ){
        // ilog("${t} ${id}",("t",tbt.block_time)("id",tbt.trace->id.str()));
        auto session = m_session_pool->get_session();
        dfs_inline_traces( session, tbt.trace->action_traces, tbt.trace->id, tbt.block_time  );
    }

    void sql_database::dfs_inline_traces( std::shared_ptr<soci::session> session, vector<chain::action_trace> trace,  chain::transaction_id_type transaction_id, chain::block_timestamp_type block_time ){
        for(auto& atc : trace){
            if( atc.receipt.receiver == atc.act.account ){
                auto is_success = m_actions_table->add( session, atc.act, transaction_id, block_time, m_action_filter_on );
                if( !is_success && atc.inline_traces.size()!=0 ){
                    dfs_inline_traces( session, atc.inline_traces, transaction_id, block_time );
                }
            }
        }
    }

    int sql_database::get_min_account_id(){

        int id = 0;           
        auto session = m_session_pool->get_session();
        *session << "select min(id) from accounts",
            soci::into(id);

        if (!session->got_data()){
            return -1;
        }  

        return id;
        
    }

    int sql_database::get_max_account_id(){

        int id = 0;           
        auto session = m_session_pool->get_session();
        *session << "select max(id) from accounts",
            soci::into(id);
        
        if (!session->got_data()){
            return -1;
        }  

        return id;
        
    }
    
    bool sql_database::update_token(std::string account){

        auto ro_api = app().get_plugin<sql_db_plugin>().get_read_only_api();
        eosio::sql_db_apis::read_only::get_hold_tokens_params param;
        param.account = name(account);
        
        eosio::sql_db_apis::read_only::get_hold_tokens_result result = ro_api.get_hold_tokens(param);
        
        for(auto it = result.tokens.begin() ; it != result.tokens.end(); it++){
            
            std::string symbol   =  it->symbol;
            std::string quantity =  it->quantity;
            int precision        =  it->precision;
            std::string contract =  it->contract.to_string();

            save_token(account,symbol,quantity,precision,contract);
        }

        return true;
    }


    bool sql_database::update_stake(std::string account){
       
        
        auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();
        if(!app().get_plugin<chain_plugin>().chain().head_block_state()){
            ilog("head block state is null"); 
            return false;
        }

       
        int64_t liquid = 0;
        int64_t staked = 0;
        int64_t unstaking = 0;
        int64_t total = 0;
        int64_t totalasset = 0;

        int64_t cpu_total = 0;
        int64_t cpu_staked = 0;
        int64_t cpu_delegated = 0;
        int64_t cpu_used = 0;
        int64_t cpu_available = 0;
        int64_t cpu_limit = 0;

        int64_t net_total = 0;
        int64_t net_staked = 0;
        int64_t net_delegated = 0;
        int64_t net_used = 0;
        int64_t net_available = 0;
        int64_t net_limit = 0;

        int64_t ram_quota = 0;
        int64_t ram_usage = 0;

        int64_t total_stake = 0;
        

        eosio::chain_apis::read_only::get_account_params param;
        param.account_name = name(account);
        eosio::chain_apis::read_only::get_account_results result = ro_api.get_account(param);

        cpu_used      = result.cpu_limit.used;
        cpu_available = result.cpu_limit.available;
        cpu_limit     = result.cpu_limit.max;


        net_used      = result.net_limit.used;
        net_available = result.net_limit.available;
        net_limit     = result.net_limit.max;

        ram_quota     = result.ram_quota;
        ram_usage     = result.ram_usage;

        if ( result.core_liquid_balance.valid() ) {
                liquid = result.core_liquid_balance->get_amount();
        }
            
        if ( result.total_resources.is_object() ) {
    
            cpu_total = asset::from_string(result.total_resources.get_object()["cpu_weight"].as_string()).get_amount();

            if( result.self_delegated_bandwidth.is_object() ) {
                
                cpu_staked = asset::from_string(result.self_delegated_bandwidth.get_object()["cpu_weight"].as_string()).get_amount();
                cpu_delegated = cpu_total - cpu_staked;

            } else {
                cpu_delegated = cpu_total;
            }
        }  

        if ( result.total_resources.is_object() ) {
            
            net_total = asset::from_string(result.total_resources.get_object()["net_weight"].as_string()).get_amount();
            if( result.self_delegated_bandwidth.is_object() ) {

                net_staked =  asset::from_string( result.self_delegated_bandwidth.get_object()["net_weight"].as_string()).get_amount();
                net_delegated = net_total - net_staked;
            }
            else {
                net_delegated = net_total;
            }
        }


        if( result.refund_request.is_object() ) {
            
            auto obj = result.refund_request.get_object();
            int net = asset::from_string( obj["net_amount"].as_string() ).get_amount();
            int cpu = asset::from_string( obj["cpu_amount"].as_string() ).get_amount();
            unstaking = net + cpu;
        }

        staked = cpu_staked + net_staked;
        total = staked + unstaking + liquid;

        eosio::chain_apis::read_only::get_table_rows_params params1;
        params1.code = name("eosio");
        params1.scope = "eosio";
        params1.limit = 1;
        params1.lower_bound = account;
        params1.table = name("voters");
        params1.table_key = "owner";
        params1.json = true;

        eosio::chain_apis::read_only::get_table_rows_result result1 = ro_api.get_table_rows(params1);
        if(result1.rows.size() == 1){
            total_stake = result1.rows[0]["staked"].as_int64();
        }

        totalasset = total_stake + unstaking + liquid;

        save_stake(account,liquid ,staked,unstaking,total,total_stake,totalasset,cpu_total,cpu_staked,cpu_delegated,cpu_used,cpu_available,cpu_limit,net_total,net_staked,net_delegated,net_used,net_available,net_limit,ram_quota,ram_usage);


        return true;

    }

    void sql_database::save_stake(
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
        ){
        
            auto session = m_session_pool->get_session();
            try{
                *session << "INSERT INTO stakes (account,liquid ,staked,unstaking,total,total_stake,totalasset,cpu_total,cpu_staked,cpu_delegated,cpu_used,cpu_available,cpu_limit,net_total,net_staked,net_delegated,net_used,net_available,net_limit,ram_quota,ram_usage)  VALUES( :account,:liquid ,:staked,:unstaking,:total,:total_stake,:totalasset,:cpu_total,:cpu_staked,:cpu_delegated,:cpu_used,:cpu_available,:cpu_limit,:net_total,:net_staked,:net_delegated,:net_used,:net_available,:net_limit,:ram_quota,:ram_usage ) ON DUPLICATE KEY UPDATE liquid=:liquid ,staked=:staked,unstaking=:unstaking,total=:total,total_stake=:total_stake,totalasset=:totalasset,cpu_total=:cpu_total,cpu_staked=:cpu_staked,cpu_delegated=:cpu_delegated,cpu_used=:cpu_used,cpu_available=:cpu_available,cpu_limit=:cpu_limit,net_total=:net_total,net_staked=:net_staked,net_delegated=:net_delegated,net_used=:net_used,net_available=:net_available,net_limit=:net_limit,ram_quota=:ram_quota,ram_usage=:ram_usage",
                        soci::use(account),
                        soci::use(liquid),
                        soci::use(staked),
                        soci::use(unstaking),
                        soci::use(total),
                        soci::use(total_stake),
                        soci::use(totalasset),
                        soci::use(cpu_total),
                        soci::use(cpu_staked),
                        soci::use(cpu_delegated),
                        soci::use(cpu_used),
                        soci::use(cpu_available),
                        soci::use(cpu_limit),
                        soci::use(net_total),
                        soci::use(net_staked),
                        soci::use(net_delegated),
                        soci::use(net_used),
                        soci::use(net_available),
                        soci::use(net_limit),
                        soci::use(ram_quota),
                        soci::use(ram_usage),
                        soci::use(liquid),
                        soci::use(staked),
                        soci::use(unstaking),
                        soci::use(total),
                        soci::use(total_stake),
                        soci::use(totalasset),
                        soci::use(cpu_total),
                        soci::use(cpu_staked),
                        soci::use(cpu_delegated),
                        soci::use(cpu_used),
                        soci::use(cpu_available),
                        soci::use(cpu_limit),
                        soci::use(net_total),
                        soci::use(net_staked),
                        soci::use(net_delegated),
                        soci::use(net_used),
                        soci::use(net_available),
                        soci::use(net_limit),
                        soci::use(ram_quota),
                        soci::use(ram_usage);

            } catch(soci::mysql_soci_error e) {
                wlog("soci::error: ${e}",("e",e.what()) );
            } catch(std::exception e) {
                wlog(" ${account} ${liquid}",("account",account)("liquid",liquid));
                wlog( "${e}",("e",e.what()) );
            } catch(...) {
                wlog(" ${account} ${liquid}",("account",account)("liquid",liquid));
            }  
    }

    void sql_database::save_token(std::string account,std::string symbol,std::string quantity,int precision,std::string contract){

        auto session = m_session_pool->get_session();
        try{
            *session << "INSERT INTO tokens (account,symbol ,balance,symbol_precision,contract_owner)  VALUES( :account, :symbol , :balance , :symbol_precision , :contract_owner ) ON DUPLICATE KEY UPDATE balance=:balance,symbol_precision=:symbol_precision",
                    soci::use(account),
                    soci::use(symbol),
                    soci::use(quantity),
                    soci::use(precision),
                    soci::use(contract),
                    soci::use(quantity),
                    soci::use(precision);

        } catch(soci::mysql_soci_error e) {
            wlog("soci::error: ${e}",("e",e.what()) );
        } catch(std::exception e) {
            wlog(" ${account} ${symbol}",("account",account)("symbol",symbol));
            wlog( "${e}",("e",e.what()) );
        } catch(...) {
            wlog(" ${account} ${symbol}",("account",account)("symbol",symbol));
        }               
    }

    bool sql_database::monitoraccount(int accountid){

        bool flag = true;
        auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();

        std::string acc_name;
        auto session = m_session_pool->get_session();

        *session << "select name from accounts where id=:id ", 
             soci::use(accountid),soci::into(acc_name);

        if(acc_name.empty()){
            return true;
        }
               
            
        flag = update_token(acc_name);
        flag = update_stake(acc_name);


        return flag;

    }

} // namespace
