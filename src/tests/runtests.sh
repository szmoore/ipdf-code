#!/bin/bash

for source in *.cpp; do
	t=${source%.cpp}
	make -C ../ tests/$t 1>>/dev/null
	(echo -n "Running $t.test ... ") 1>&2
	./$t.test 2>$t.err 1>$t.out
	if [ $? -ne 0 ]; then
		(echo " FAILURE! Press enter to see stderr") 1>&2
		read
		less $t.err
		(echo "Stopping tests after failure.") 1>&2
		exit 1
	fi
	(echo "Success!") 1>&2
done
