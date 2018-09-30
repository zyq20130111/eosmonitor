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
    
    void sql_database::update_token(std::string account){

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
    }


    void sql_database::update_stake(std::string account){
       
        auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();

        int liquid = 0;
        int staked = 0;
        int unstaking = 0;
        int total = 0;
        int totalasset = 0;

        int cpu_total = 0;
        int cpu_staked = 0;
        int cpu_delegated = 0;
        int cpu_used = 0;
        int cpu_available = 0;
        int cpu_limit = 0;

        int net_total = 0;
        int net_staked = 0;
        int net_delegated = 0;
        int net_used = 0;
        int net_available = 0;
        int net_limit = 0;

        int ram_quota = 0;
        int ram_usage = 0;

        int total_stake = 0;
        
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


    }

    void sql_database::save_stake(
        std::string account,
        int liquid,
        int staked,
        int unstaking,
        int total,
        int total_stake,
        int totalasset,
        int cpu_total,
        int cpu_staked,
        int cpu_delegated,
        int cpu_used,
        int cpu_available,
        int cpu_limit,
        int net_total,
        int net_staked,
        int net_delegated,
        int net_used,
        int net_available,
        int net_limit,
        int ram_quota,
        int ram_usage
        ){
        
            auto session = m_session_pool->get_session();
            try{
                *session << "REPLACE INTO stakes (account,liquid ,staked,unstaking,total,total_stake,totalasset,cpu_total,cpu_staked,cpu_delegated,cpu_used,cpu_available,cpu_limit,net_total,net_staked,net_delegated,net_used,net_available,net_limit,ram_quota,ram_usage)  VALUES( :account,:liquid ,:staked,:unstaking,:total,:total_stake,:totalasset,:cpu_total,:cpu_staked,:cpu_delegated,:cpu_used,:cpu_available,:cpu_limit,:net_total,:net_staked,:net_delegated,:net_used,:net_available,:net_limit,:ram_quota,:ram_usage ) ",
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
            *session << "INSERT INTO tokens (account,symbol ,balance,symbol_precision,contract_owner)  VALUES( :from, :receiver , :stake_net_quantity , :stake_cpu_quantity , :tran_id ) ",
                    soci::use(account),
                    soci::use(symbol),
                    soci::use(quantity),
                    soci::use(precision),
                    soci::use(contract);

        } catch(soci::mysql_soci_error e) {
            wlog("soci::error: ${e}",("e",e.what()) );
        } catch(std::exception e) {
            wlog(" ${account} ${symbol}",("account",account)("symbol",symbol));
            wlog( "${e}",("e",e.what()) );
        } catch(...) {
            wlog(" ${account} ${symbol}",("account",account)("symbol",symbol));
        }               
    }

    void sql_database::monitoraccount(int accountid){

        auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();

        std::string acc_name;
        auto session = m_session_pool->get_session();

        *session << "select name from accounts where id=:id ", 
             soci::use(accountid),soci::into(acc_name);
               
        try{
            
            update_token(acc_name);
            update_stake("accountnum11");

        } catch(fc::exception& e) {
            wlog("${e}",("e",e.what()));
        } catch(std::exception& e) {
            wlog("${e}",("e",e.what()));
        } catch (...) {
            wlog("unknown");
        }

    }

} // namespace
