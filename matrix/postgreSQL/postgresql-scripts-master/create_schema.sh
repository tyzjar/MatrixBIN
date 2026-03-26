#!/bin/bash -exu

dir="$(dirname $0)"
database="$1"
username="$2"

if [ -z "$database" ] ; then
    echo "Usage: $0 [connection info]"
    echo "   for example, $0 postgresql://mcal@localhost/mcaldb"
    exit 1
fi

sudo -u postgres psql $database -U $username -W -f "./create_schema.sql"
