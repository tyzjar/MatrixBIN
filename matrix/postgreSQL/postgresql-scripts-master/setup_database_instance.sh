#!/bin/bash -ex

# Note that this script creates a database user without a password.
# This is suitable only for development on your own single-user computer.
# Do not use this script as such for any production databases.

dir="$(dirname $0)"
database="$1"
username="$2"
password="$3"

if [ -z "$database" ] || [ -z "$username" ] ; then
    echo "Usage: $0 [database] [username] [password]"
    exit 1
fi

# Usually you would like to run this as the postgres user.

# However, if you use this database for development work with a
# postgresql server running as yourself, you do not need the sudo
# statement, which is currently commented out.

# sudo -u postgres
sudo -u postgres createuser $username

sudo -u postgres psql postgres <<EOF
ALTER USER $username WITH LOGIN PASSWORD '$password' \gexec
EOF

sudo -u postgres createdb $database --owner $username

# SELECT 'CREATE ROLE $username' WHERE NOT EXISTS (SELECT FROM pg_roles WHERE rolname = '$username') \gexec
# SELECT 'CREATE DATABASE $database WITH OWNER $username' WHERE NOT EXISTS (SELECT FROM pg_database WHERE datname = '$database') \gexec

