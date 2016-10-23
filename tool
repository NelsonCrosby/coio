#!/bin/sh
set -e
case "$1" in
  test)
    shift
    lua 'test/all.lua' "$@"
    ;;
  *)
    luarocks --tree=. "$@"
    ;;
esac
