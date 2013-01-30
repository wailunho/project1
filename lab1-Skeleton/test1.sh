
#testing for ;
#clear; ls; echo good			#passed

#testing < and >
#cat < test3 > test2			#passed

#testing |
#ls | sort						#failed

#testing &&
#true && false && ls 			#passed

#testing ||
#-1 || -1 || -1 || ls		    #passed

#testing for multiple commands	#passed
#echo hi
#echo hello

#: : : 							#failed

#testing for subshell command
#(echo 11222 > test2	)			#failed (read-command.c)

