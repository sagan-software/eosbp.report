TODO

- [x] Fetch logos
- [ ] Ping nodes
- [ ] Generate reports
  - [ ] Validate `bs.json`
  - [ ] Validate `regproducer`
- [x] Fetch all producers from regproducer if 'more' is true

curl http://node2.liquideos.com/v1/chain/get_table_rows -X POST -d '{"scope":"eosio", "code":"eosio", "table":"voters", "json": true, "lower_bound":0, "upper_bound":-1, "limit":10}'

{
"max_block_net_usage":1048576,
"target_block_net_usage_pct":1000,
"max_transaction_net_usage":524288,
"base_per_transaction_net_usage":12,
"net_usage_leeway":500,
"context_free_discount_net_usage_num":20,
"context_free_discount_net_usage_den":100,
"max_block_cpu_usage":200000,
"target_block_cpu_usage_pct":1000,
"max_transaction_cpu_usage":150000,
"min_transaction_cpu_usage":100,
"max_transaction_lifetime":3600,
"deferred_trx_expiration_window":600,
"max_transaction_delay":3888000,
"max_inline_action_size":4096,
"max_inline_action_depth":4,
"max_authority_depth":6,
"max_ram_size":"68719476736",
"total_ram_bytes_reserved":"52375348962",
"total_ram_stake":"32046076637",
"last_producer_schedule_update":"2018-07-16T04:14:59.500",
"last_pervote_bucket_fill":"1531713928500000",
"pervote_bucket":247865933,
"perblock_bucket":41344811,
"total_unpaid_blocks":106343,
"total_activated_stake":"3093873871383",
"thresh_activated_stake_time":"1529505892000000",
"last_producer_schedule_size":21,
"total_producer_vote_weight":"10298164117186230272.00000000000000000",
"last_name_close":"2018-07-11T15:27:03.500"
}

if( \_gstate.total_unpaid_blocks > 0 ) {
producer_per_block_pay = (\_gstate.perblock_bucket _ prod.unpaid_blocks) / \_gstate.total_unpaid_blocks;
41344811 _ 5612 / 106343
}

if( \_gstate.total_producer_vote_weight > 0 ) {
producer_per_vote_pay = (\_gstate.pervote_bucket _ prod.total_votes ) / \_gstate.total_producer_vote_weight);
247865933 _ 282133417800890976 / 10298164117186230272 + 41344811 \* 5612 / 106343
}

min_pervote_daily_pay = 1000000;

if( producer_per_vote_pay < min_pervote_daily_pay ) {
producer_per_vote_pay = 0;
}

producer vote pay = pervote_bucket \* total_votes / total_producer_vote_weight
if producer vote pay < 1000000 then 0

producer block pay = perblock_bucket \* unpaid_blocks / total_unpaid_blocks

estimated producer pay in EOS = (producer vote pay + producer block pay) \* 10000
