#!/bin/bash -x

dir=`pwd`

db_user=kemek
password=kemek
db_name=scaledb
db_connstring="postgresql://$db_user@localhost/$db_name"

#killall postgres
/usr/lib/postgresql/14/bin/pg_ctl -D "$dir/local-database/" stop
#sudo systemctl stop postgresql
rm -rf "$dir/local-database" "$dir/psql.log"
./create_postgresql_instance.sh "$dir/local-database/"
./start_db.sh "$dir/local-database/" "$dir/psql.log"
./setup_database_instance.sh "$db_name" "$db_user" "$password"
./create_schema.sh "$db_connstring" "$db_user"
