/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

#pragma once

#include <boost/noncopyable.hpp>
#include <boost/chrono.hpp>
#include <boost/signals2/connection.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <eosio/chain/block_state.hpp>
#include <eosio/chain/transaction.hpp>
#include <fc/log/logger.hpp>
#include <eosio/sql_db_plugin/database.hpp>
#include <eosio/chain_plugin/chain_plugin.hpp>
// #include "database.hpp"

namespace eosio {

class consumer final : public boost::noncopyable {
    public:
        consumer(std::unique_ptr<sql_database> db, size_t queue_size);
        ~consumer();
        void shutdown();

        template<typename Queue, typename Entry>
        void queue(boost::mutex&, boost::condition_variable&, Queue&, const Entry&, size_t );

        void push_transaction_metadata( const chain::transaction_metadata_ptr& );
        void push_block_state( const chain::block_state_ptr& );
        void run_blocks();
        void run_monitors();

        std::deque<chain::block_state_ptr> block_state_queue;
        std::deque<chain::block_state_ptr> block_state_process_queue;
        std::deque<chain::transaction_metadata_ptr> transaction_metadata_queue;
        std::deque<chain::transaction_metadata_ptr> transaction_metadata_process_queue;

        std::unique_ptr<sql_database> db;
        size_t queue_size;
        boost::atomic<bool> exit{false};
        boost::thread consume_thread_run_blocks;
        boost::mutex mtx_blocks;

        boost::thread consume_thread_run_monitors;
        boost::mutex mtx_monitors;
        boost::condition_variable condition;

        int min_account_id;
        int max_account_id;
        int cur_account_id;
        bool start_loop;
        bool is_waiting;


    };

    consumer::consumer(std::unique_ptr<sql_database> db, size_t queue_size):
        db(std::move(db)),
        queue_size(queue_size),
        exit(false),
        min_account_id(0),
        cur_account_id(0),
        start_loop(false),
        is_waiting(true),
        consume_thread_run_blocks(boost::thread([&]{this->run_blocks();})),
        consume_thread_run_monitors(boost::thread([&]{this->run_monitors();}))
        { }

    consumer::~consumer() {
        exit = true;
        condition.notify_all();
        consume_thread_run_blocks.join();
        consume_thread_run_monitors.join();
    }

    void consumer::shutdown() {
        exit = true;
        condition.notify_all();
        consume_thread_run_blocks.join();
        consume_thread_run_monitors.join();
    }

    template<typename Queue, typename Entry>
    void consumer::queue(boost::mutex& mtx, boost::condition_variable& condition, Queue& queue, const Entry& e, size_t queue_size) {
        int sleep_time = 100;
        size_t last_queue_size = 0;
        boost::mutex::scoped_lock lock(mtx);
        if (queue.size() > queue_size) {
            lock.unlock();
            condition.notify_one();
            if (last_queue_size < queue.size()) {
                sleep_time += 100;
            } else {
                sleep_time -= 100;
                if (sleep_time < 0) sleep_time = 100;
            }
            last_queue_size = queue.size();
            boost::this_thread::sleep_for(boost::chrono::milliseconds(sleep_time));
            lock.lock();
        }
        queue.emplace_back(e);
        lock.unlock();
        condition.notify_all();
    }

    void consumer::push_block_state( const chain::block_state_ptr& bs ){
        try {
            queue(mtx_blocks, condition, block_state_queue, bs, queue_size);
        } catch (fc::exception& e) {
            elog("FC Exception while accepted_block ${e}", ("e", e.to_string()));
        } catch (std::exception& e) {
            elog("STD Exception while accepted_block ${e}", ("e", e.what()));
        } catch (...) {
            elog("Unknown exception while accepted_block");
        }
    }

    void consumer::run_blocks() {
        ilog("Consumer thread Start run_blocks");
        while (!exit) { 
            try{
                boost::mutex::scoped_lock lock(mtx_blocks);
                while(block_state_queue.empty() && !exit){
                    condition.wait(lock);
                }

                size_t block_state_size = block_state_queue.size();
                if( block_state_size > 0 ){
                    block_state_process_queue = std::move(block_state_queue);
                }

                lock.unlock();

                if( block_state_size > (queue_size * 0.75)) {
                    wlog("reversible queue size: ${q}", ("q", block_state_size));
                } else if (exit) {
                    ilog("reversible draining queue, size: ${q}", ("q", block_state_size));
                }          

                // process blocks
                while (!block_state_process_queue.empty()) {
                    const auto& bs = block_state_process_queue.front();
                    try{
                        db->consume_block_state( bs );
                    } catch (fc::exception& e) {
                        elog("FC Exception while consuming block ${e}", ("e", e.to_string()));
                    } catch (std::exception& e) {
                        elog("STD Exception while consuming block ${e}", ("e", e.what()));
                    } catch (...) {
                        elog("Unknown exception while consuming block");
                    } 
                    block_state_process_queue.pop_front();
                }

                condition.notify_all();
            } catch (std::exception& e) {
                elog("lose some catch ${e}", ("e", e.what()));
            } catch (...) {
                elog("Unknown exception while consuming block");
            }  

        }
        
        ilog("Consumer thread End run_blocks");
    }

    void consumer::run_monitors() {

        ilog("Consumer thread Start run_monitors");
        while (!exit) { 
                           
            if(is_waiting){
                boost::this_thread::sleep(boost::posix_time::seconds(10));
                is_waiting = false;
            }
            

            //开始求得第一条和最后一条记录的ID
            if(!start_loop){
            
                min_account_id = db->get_min_account_id();
                max_account_id = db->get_max_account_id();
                cur_account_id = min_account_id;
                start_loop = true;
            
            }

            //开始循环从第一条记录
            if(cur_account_id >= 0){

                if(db->monitoraccount(cur_account_id))
                {
                    cur_account_id = cur_account_id + 1;

                    if(cur_account_id > max_account_id){
                        start_loop = false;
                    }
                }
            }

            boost::this_thread::sleep(boost::posix_time::milliseconds(10));

        }
        
        ilog("Consumer thread End run_monitors");
    }



} // namespace

