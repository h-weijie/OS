pkill '^work*'
./remove;
for name in 'create' 'remove' 'workA' 'workB' 'workC'
do
	gcc work5.c -o $name
done
./create;
./workA & ./workB & ./workC;
