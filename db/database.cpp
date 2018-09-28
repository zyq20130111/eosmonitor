// #include "database.hpp"
#include <eosio/sql_db_plugin/database.hpp>

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

    void sql_database::monitoraccount(int accountid){
        
        std::string acc_name;
        auto session = m_session_pool->get_session();

        *session << "select name from accounts where id=:id ", 
             soci::use(accountid),soci::into(acc_name);
        
        auto assets = m_actions_table->get_assets(m_session_pool->get_session(), 0, 20);
        /*
        for(auto it = assets.begin() ; it != assets.end(); it++){
            try{
                
                std::string contract = it->get<string>(0);
                std::string symbol = it->get<string>(3);

                const abi_def abi = get_abi( db, contract );
                auto table_type = get_table_type( abi, "accounts" );

                walk_key_value_table(t.contract, p.account, N(accounts), [&](const key_value_object& obj){
                    EOS_ASSERT( obj.value.size() >= sizeof(asset), chain::asset_type_exception, "Invalid data on table");

                    asset cursor;
                    fc::datastream<const char *> ds(obj.value.data(), obj.value.size());
                    fc::raw::unpack(ds, cursor);

                    EOS_ASSERT( cursor.get_symbol().valid(), chain::asset_type_exception, "Invalid asset");

                    if( cursor.symbol_name() == t.symbol ) {
                        t.quantity = asset_amount_to_string(cursor);
                        t.precision = cursor.decimals();
                        result.tokens.emplace_back(t);
                    }

                    // return false if we are looking for one and found it, true otherwise
                    return !(cursor.symbol_name() == t.symbol);

                }, [&](){
                    asset cursor = asset(0, chain::symbol(chain::string_to_symbol(it->get<int>(2),t.symbol.c_str())));
                    t.quantity = asset_amount_to_string( cursor );
                    t.precision = it->get<int>(2);
                    result.tokens.emplace_back(t);
                });
            } catch(fc::exception& e) {
                wlog("${e}",("e",e.what()));
            } catch(std::exception& e) {
                wlog("${e}",("e",e.what()));
            } catch (...) {
                wlog("unknown");
            }
        }*/

    }

} // namespace
