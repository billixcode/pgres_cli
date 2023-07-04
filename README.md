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


./table db:testdb:Cars:cr:"Id INTEGER PRIMARY KEY, Name VARCHAR(20), Price INT"
./table db:testdb:sales:cr:"cust varchar(20), prod varchar(20), day integer, month integer, year integer, state char(2), quant integer, date date"