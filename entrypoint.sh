#!/bin/sh

shutdown_handler()
{
    kill -s SIGINT $PID
}

cd /app/
su tw_server -s /bin/sh -c "$@" &
PID=$!

trap shutdown_handler SIGINT SIGTERM SIGKILL

wait $PID
