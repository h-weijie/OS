pkill '^producer$'
pkill '^consumer$'
for name in 'create' 'remove' 'producer' 'consumer'
do
	gcc work3.c -o $name
done
./remove;
./create;
./producer & ./consumer;
