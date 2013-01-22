NODE=node

rm -f repo.ior test.log
DCPSInfoRepo &
until test -f repo.ior; do sleep 1; done

$NODE test.js &

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:idl ./test_publisher
kill %1
wait %1
