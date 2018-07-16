TODO

- [x] Fetch logos
- [ ] Ping nodes
- [ ] Generate reports
  - [ ] Validate `bs.json`
  - [ ] Validate `regproducer`
- [x] Fetch all producers from regproducer if 'more' is true

curl http://node2.liquideos.com/v1/chain/get_table_rows -X POST -d '{"scope":"eosio", "code":"eosio", "table":"voters", "json": true, "lower_bound":0, "upper_bound":-1, "limit":10}'
