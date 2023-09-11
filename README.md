https://zetcode.com/db/postgresqlc/
https://stackoverflow.com/questions/13086073/rails-install-pg-cant-find-the-libpq-fe-h-header


gcc libpq.c -o libpq -I /usr/include/postgresql -lpq -std=c99
gcc server_version.c -o server_version -I /usr/include/postgresql -lpq -std=c99
gcc create_table.c -o create_table -I /usr/include/postgresql -lpq -std=c99
gcc table.c -o table -I /usr/include/postgresql -lpq -std=c99

/etc/postgresql/15/main$ sudo nano pg_hba.conf

ALTER USER postgres PASSWORD 'postgres';

psql -U postgres -W
\c testdb


// compile
 gcc pg_cli.c -o pg_cli -I /usr/include/postgresql -lpq -std=c99 -w

## Comands
### create db

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

