pkill '^work*'
for name in 'create' 'remove' 'workA' 'workB' 'workC'
do
	gcc $1 -o $name
done
./remove;
./create;
./workA & ./workB & ./workC;
