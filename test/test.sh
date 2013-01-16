rm repo.ior
DCPSInfoRepo &
until test -f repo.ior; do sleep 1; done
node test.js &
./test_publisher
kill %1
wait %1
