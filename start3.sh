pkill '^producer$'
pkill '^consumer$'
for name in 'create0' 'remove0' 'producer' 'consumer'
do
	gcc work3.c -o $name
done
./remove0;
./create0;
./producer & ./consumer;
