# About
This is a simple postgres commandline utility that wraps the libpq C library

https://www.postgresql.org/docs/9.5/libpq.html

## Install
### Postgres
//TODO
### libfq install
//TODO

## Compile
gcc pg_cli.c -o pg_cli -I /usr/include/postgresql -lpq -std=c99 -w

## Comands
### create db
//TODO

### create table
./pg_cli -d testdb -o cre -m 'Id INTEGER PRIMARY KEY, Name VARCHAR(20), Price INT' -t homes -v

### insert data
./pg_cli -d testdb -o ins -s "28,'Bill Jonx',78000" -t homes -v

### bulk daata insert
./pg_cli -d testdb -o fle -s homes_load.sql -t homes

### query
./pg_cli -d testdb -s "select count(owner) from homes" -t homes

### drop table
./pg_cli -d testdb -o del -t homes -v

