pkill '^work*'
for name in 'create' 'remove' 'workA' 'workB' 'workC'
do
	gcc work5.c -o $name
done
./remove;
./create;
./workA & ./workB & ./workC;
